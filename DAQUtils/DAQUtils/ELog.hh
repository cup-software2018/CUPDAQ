#pragma once
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h> // isatty

#ifndef LOG_COMPILE_LEVEL
// 0=INFO, 1=WARN, 2=ERROR, 3=DEBUG, 4=STATS
#define LOG_COMPILE_LEVEL 0
#endif

enum class ELOGLEVEL : int { Info = 0, Warn, Error, Debug, Stats };

class ELog {
public:
  struct Options {
    ELOGLEVEL runtime_level = ELOGLEVEL::Info;
    bool use_color = true;
    bool to_stderr = false;
    std::optional<std::string> file_path = std::nullopt;
    std::string time_fmt = "%Y-%m-%d %H:%M:%S";
  };

  static ELog & instance()
  {
    static ELog s;
    return s;
  }

  void configure(const Options & opt)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _opt = opt;
    _runtime_level.store(static_cast<int>(_opt.runtime_level), std::memory_order_relaxed);
    _open_file_unlocked();
  }

  void set_level(ELOGLEVEL lv)
  {
    _runtime_level.store(static_cast<int>(lv), std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(_mutex);
    _opt.runtime_level = lv;
  }

  ELOGLEVEL level() const
  {
    return static_cast<ELOGLEVEL>(_runtime_level.load(std::memory_order_relaxed));
  }

  // ---------- printf-style logging ----------
  void logf(ELOGLEVEL lv, const char * pretty_func, const char * fmt, ...)
  {
    if ((int)lv < LOG_COMPILE_LEVEL) return;
    if ((int)lv < _runtime_level.load(std::memory_order_relaxed)) return;

    va_list ap;
    va_start(ap, fmt);
    std::string msg = _vformat(fmt, ap);
    va_end(ap);

    _emit_line(lv, pretty_func, msg);
  }

private:
  ELog()
  {
    _is_tty = ::isatty(1);
    _runtime_level.store(static_cast<int>(_opt.runtime_level), std::memory_order_relaxed);
  }

  ~ELog() = default;

  std::string _vformat(const char * fmt, va_list ap)
  {
    char sbuf[1024];
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(sbuf, sizeof(sbuf), fmt, ap2);
    va_end(ap2);
    if (n >= 0 && static_cast<size_t>(n) < sizeof(sbuf)) return std::string(sbuf, n);

    size_t size = (n > 0) ? (n + 1) : (sizeof(sbuf) * 2);
    std::string out(size, '\0');
    va_list ap3;
    va_copy(ap3, ap);
    n = vsnprintf(out.data(), size, fmt, ap3);
    va_end(ap3);
    if (n < 0) return "";
    out.resize(n);
    return out;
  }

  void _emit_line(ELOGLEVEL lv, const char * pretty_func, const std::string & msg)
  {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&t, &tm_buf);

    std::string where = _extract_class_method(pretty_func);

    std::ostringstream header;
    header << std::put_time(&tm_buf, _opt.time_fmt.c_str()) << "::[" << _level_tag(lv) << "] " << where << ": ";

    std::string line = header.str() + msg;

    std::lock_guard<std::mutex> lock(_mutex);
    _output_line(lv, line);
  }

  void _output_line(ELOGLEVEL lv, const std::string & line)
  {
    // console
    if (_opt.use_color && _is_tty) {
      (_opt.to_stderr ? std::cerr : std::cout) << _color_open(lv) << line << _color_close() << '\n';
    }
    else {
      (_opt.to_stderr ? std::cerr : std::cout) << line << '\n';
    }

    // file (no rotation)
    if (_file) {
      (*_file) << line << '\n';
      _file->flush();
    }
  }

  std::string _extract_class_method(const std::string & pretty) const
  {
    static const std::regex re(R"(([\w:]+)::([\w<>~]+)\s*\(|([\w<>~]+)\s*\()");
    std::smatch m;
    if (std::regex_search(pretty, m, re)) {
      if (m[1].matched && m[2].matched) return m[1].str() + "::" + m[2].str();
      if (m[3].matched) return m[3].str();
    }
    return pretty;
  }

  const char * _level_tag(ELOGLEVEL lv) const
  {
    switch (lv) {
      case ELOGLEVEL::Info: return "INFO";
      case ELOGLEVEL::Warn: return "WARN";
      case ELOGLEVEL::Error: return "ERROR";
      case ELOGLEVEL::Debug: return "DEBUG";
      case ELOGLEVEL::Stats: return "STATS";
      default: return "UNKNOWN";
    }
  }

  const char * _color_open(ELOGLEVEL lv) const
  {
    switch (lv) {
      case ELOGLEVEL::Info: return "\033[32m";
      case ELOGLEVEL::Warn: return "\033[33m";
      case ELOGLEVEL::Error: return "\033[31m";
      case ELOGLEVEL::Debug: return "\033[36m";
      case ELOGLEVEL::Stats: return "\033[37m";
      default: return "";
    }
  }

  const char * _color_close() const { return "\033[0m"; }

  void _open_file_unlocked()
  {
    _file.reset();
    if (!_opt.file_path) return;
    _file.emplace(*_opt.file_path, std::ios::app);
  }

  Options _opt{};
  std::optional<std::ofstream> _file;
  bool _is_tty = true;
  mutable std::mutex _mutex;
  std::atomic<int> _runtime_level{static_cast<int>(ELOGLEVEL::Info)};
};

// ---------- printf-style macros ----------
#define INFO(fmt, ...) ELog::instance().logf(ELOGLEVEL::Info, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define WARNING(fmt, ...) ELog::instance().logf(ELOGLEVEL::Warn, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) ELog::instance().logf(ELOGLEVEL::Error, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) ELog::instance().logf(ELOGLEVEL::Debug, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#define STATS(fmt, ...) ELog::instance().logf(ELOGLEVEL::Stats, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)

inline void init_elog(const ELog::Options & opt = {}) { ELog::instance().configure(opt); }
