#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

template <class T>
class ConcurrentDeque {
public:
  ConcurrentDeque() = default;
  ConcurrentDeque(const ConcurrentDeque &) = delete;
  ConcurrentDeque & operator=(const ConcurrentDeque &) = delete;

  // move constructor
  ConcurrentDeque(ConcurrentDeque && other) noexcept
  {
    std::lock_guard<std::mutex> lock(other._mutex);
    _queue = std::move(other._queue);
    _stop = other._stop;
  }

  // move assignment
  ConcurrentDeque & operator=(ConcurrentDeque && other) noexcept
  {
    if (this == &other) return *this;

    std::scoped_lock lock(_mutex, other._mutex); // C++17
    _queue = std::move(other._queue);
    _stop = other._stop;
    return *this;
  }

  // ------------------------------------------------------------
  // Push operations
  // ------------------------------------------------------------
  // Push to the back (copy)
  void push_back(const T & v)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_back(v);
    }
    _cv.notify_one();
  }

  // Push to the back (move)
  void push_back(T && v)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_back(std::move(v));
    }
    _cv.notify_one();
  }

  // Push to the front (copy)
  void push_front(const T & v)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_front(v);
    }
    _cv.notify_one();
  }

  // Push to the front (move)
  void push_front(T && v)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_front(std::move(v));
    }
    _cv.notify_one();
  }

  // Construct element in-place at back
  template <class... Args>
  T & emplace_back(Args &&... args)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto & ref = _queue.emplace_back(std::forward<Args>(args)...);
    _cv.notify_one();
    return ref;
  }

  // Construct element in-place at front
  template <class... Args>
  T & emplace_front(Args &&... args)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto & ref = _queue.emplace_front(std::forward<Args>(args)...);
    _cv.notify_one();
    return ref;
  }

  // ------------------------------------------------------------
  // Destructive pop (immediate or with timeout)
  // Returns optional<T>
  // ------------------------------------------------------------

  // Pop from front (non-blocking or timed wait)
  std::optional<T> pop_front(std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
  {
    // Non-blocking
    if (timeout == std::chrono::milliseconds(0)) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return std::nullopt;
      T v = std::move(_queue.front());
      _queue.pop_front();
      return v;
    }
    // Blocking wait until element available or timeout
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return std::nullopt;
      if (_stop && _queue.empty()) return std::nullopt;
      T v = std::move(_queue.front());
      _queue.pop_front();
      return v;
    }
  }

  // Pop from back (non-blocking or timed wait)
  std::optional<T> pop_back(std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
  {
    // Non-blocking
    if (timeout == std::chrono::milliseconds(0)) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return std::nullopt;
      T v = std::move(_queue.back());
      _queue.pop_back();
      return v;
    }
    // Blocking wait
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return std::nullopt;
      if (_stop && _queue.empty()) return std::nullopt;
      T v = std::move(_queue.back());
      _queue.pop_back();
      return v;
    }
  }

  // ------------------------------------------------------------
  // Non-destructive peek front/back
  // Returns optional<T> (copies)
  // ------------------------------------------------------------

  // Peek front (non-blocking or timed wait)
  std::optional<T> front(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
  {
    if (timeout == std::chrono::milliseconds(0)) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return std::nullopt;
      return _queue.front(); // copy
    }
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return std::nullopt;
      if (_stop && _queue.empty()) return std::nullopt;
      return _queue.front(); // copy
    }
  }

  // Peek back (non-blocking or timed wait)
  std::optional<T> back(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
  {
    if (timeout == std::chrono::milliseconds(0)) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return std::nullopt;
      return _queue.back(); // copy
    }
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return std::nullopt;
      if (_stop && _queue.empty()) return std::nullopt;
      return _queue.back(); // copy
    }
  }

  // ------------------------------------------------------------
  // Non-destructive front pointer access
  // Only enabled when T has element_type (e.g. std::unique_ptr<U>)
  // ------------------------------------------------------------
  template <class U = T, class = std::void_t<typename U::element_type>>
  auto front_ptr(std::chrono::milliseconds timeout = std::chrono::milliseconds{0}) const -> typename U::element_type *
  {
    if (timeout == std::chrono::milliseconds{0}) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return nullptr;
      return _queue.front().get();
    }
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return nullptr;
      if (_stop && _queue.empty()) return nullptr;
      return _queue.front().get();
    }
  }

  template <class U = T, class = std::void_t<typename U::element_type>>
  auto back_ptr(std::chrono::milliseconds timeout = std::chrono::milliseconds{0}) const -> typename U::element_type *
  {
    if (timeout == std::chrono::milliseconds{0}) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (_queue.empty()) return nullptr;
      return _queue.back().get();
    }
    else {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!_cv.wait_for(lock, timeout, [&] { return _stop || !_queue.empty(); })) return nullptr;
      if (_stop && _queue.empty()) return nullptr;
      return _queue.back().get();
    }
  }

  // ------------------------------------------------------------
  // Introspection
  // ------------------------------------------------------------

  // Check if empty
  bool empty() const
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.empty();
  }

  // Get number of elements
  std::size_t size() const
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
  }

  // Remove all elements
  void clear()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.clear();
  }

  // Shrink memory usage
  void shrink_to_fit()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.shrink_to_fit();
  }

  // ------------------------------------------------------------
  // Shutdown control
  // ------------------------------------------------------------

  // Stop all waiting threads
  void stop()
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _stop = true;
    }
    _cv.notify_all();
  }

  // Resume queue operations
  void restart()
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _stop = false;
  }

  // Check if stopped
  bool is_stopped() const
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _stop;
  }

private:
  mutable std::mutex _mutex;
  mutable std::condition_variable _cv;
  std::deque<T> _queue;
  bool _stop{false};
};
