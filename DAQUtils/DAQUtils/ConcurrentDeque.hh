#ifndef CONCURRENTDEQUE_hh
#define CONCURRENTDEQUE_hh

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

template <typename T>
class ConcurrentDeque {

public:
  template <typename... Args>
  void push_front(Args &&... args)
  {
    std::unique_lock<std::mutex> lock{_mutex};
    _collection.emplace_front(std::forward<Args>(args)...);
    lock.unlock();
    _condNewData.notify_one();
  }

  template <typename... Args>
  void push_back(Args &&... args)
  {
    std::unique_lock<std::mutex> lock{_mutex};
    _collection.emplace_back(std::forward<Args>(args)...);
    lock.unlock();
    _condNewData.notify_one();
  }

  void clear(void)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _collection.clear();
  }

  bool empty(void)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _collection.empty();
  }

  int size(void)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _collection.size();
  }

  void shrink_to_fit(void)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _collection.shrink_to_fit();
  }

  T popfront(bool wait = true, int time = 1000) noexcept
  {
    std::unique_lock<std::mutex> lock{_mutex};

    if (_collection.empty()) {
      if (wait) {
        if (_condNewData.wait_for(lock, std::chrono::milliseconds(time)) ==
            std::cv_status::timeout) {
          return nullptr;
        }
      }
      else {
        return nullptr;
      }
    }
    auto elem = std::move(_collection.front());
    _collection.pop_front();

    return elem;
  }

  T front(bool wait = true, int time = 1000) noexcept
  {
    std::unique_lock<std::mutex> lock{_mutex};

    if (_collection.empty()) {
      if (wait) {
        if (_condNewData.wait_for(lock, std::chrono::milliseconds(time)) ==
            std::cv_status::timeout) {
          return nullptr;
        }
      }
      else {
        return nullptr;
      }
    }
    auto elem = std::move(_collection.front());
    return elem;
  }

  void pop_front(void) noexcept
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _collection.pop_front();
  }

private:
  std::deque<T> _collection;
  std::mutex _mutex;
  std::condition_variable _condNewData;
};

#endif // CONCURRENT_DEQUE_H
