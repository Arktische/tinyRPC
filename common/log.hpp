//
// Created by tyx on 12/23/21.
//
#pragma once
#include <chrono>
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP

#include <cinttypes>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#include "fastclock.hpp"
#include "noncopyable.hpp"
#include "singleton.hpp"
#include "string.hpp"
using cstr = const char* const;

static constexpr cstr past_last_slash(cstr str, cstr last_slash) {
  return *str == '\0'  ? last_slash
         : *str == '/' ? past_last_slash(str + 1, str + 1)
                       : past_last_slash(str + 1, last_slash);
}

static constexpr cstr past_last_slash(cstr str) {
  return past_last_slash(str, str);
}
#define __SHORT_FILE__                              \
  ({                                                \
    constexpr cstr sf__{past_last_slash(__FILE__)}; \
    sf__;                                           \
  })

#define LOG(level) common::LogMessage<level>(__SHORT_FILE__, __LINE__).stream()

enum Level { GLOBAL, DEBUG, INFO, WARN, ERROR, FATAL };
template <Level LEVEL>
struct LogLv;

template <>
struct LogLv<GLOBAL> {
  static void* output;
};
void* LogLv<GLOBAL>::output{stdout};

template <>
struct LogLv<DEBUG> {
  static char id[8];
  static void* output;
};
char LogLv<DEBUG>::id[8]{"DEBUG"};
void* LogLv<DEBUG>::output{nullptr};

template <>
struct LogLv<INFO> {
  static char id[8];
  static void* output;
};
char LogLv<INFO>::id[8]{"INFO"};
void* LogLv<INFO>::output{nullptr};

template <>
struct LogLv<WARN> {
  static char id[8];
  static void* output;
};
char LogLv<WARN>::id[8]{"WARN"};
void* LogLv<WARN>::output{nullptr};

template <>
struct LogLv<ERROR> {
  static char id[8];
  static void* output;
};
char LogLv<ERROR>::id[8]{"ERROR"};
void* LogLv<ERROR>::output{nullptr};

template <>
struct LogLv<FATAL> {
  static char id[8];
  static void* output;
};
char LogLv<FATAL>::id[8]{"FATAL"};
void* LogLv<FATAL>::output{nullptr};

namespace common {

using std::cout;
using std::size_t;
using std::string;
static const int kSmallBufferSize = 1024;
static const int kMaxNumericSize = 48;

// thread local buffer for LogStream
template <size_t SIZE, Level LV>
static thread_local char gLogBuf[64 + SIZE] = {0};

// using placement new to construct LogStream object on its memory.
template <size_t SIZE, Level LV>
static thread_local std::aligned_storage<SIZE> gObjCache;

// LogStreamBuffer fixed size buffer
template <size_t SIZE>
class LogStreamBuffer : NonCopyable {
 public:
  explicit LogStreamBuffer(char* buf) : data_(buf), cur_(buf) {}
  char* data() { return data_; }
  inline int size() { return cur_ - data_; }
  inline int avail() { return static_cast<int>(data_ + SIZE - cur_); }
  void append(const char* buf, size_t len) {
    if (static_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  void appendfx(const std::function<size_t(char*, size_t)>& f) {
    cur_ += f(cur_, avail());
  }

  template <class T>
  void appendx(T& d) {
    memcpy(cur_, static_cast<const char*>(&d), sizeof(T));
    cur_ += sizeof(T);
  }
  template <class T>
  void appendx(T* d) {
    memcpy(cur_, static_cast<const char*>(d), sizeof(T));
    cur_ += sizeof(T);
  }

 private:
  char* cur_;
  char* data_;
};

// LogStream supports stream operator & output
class LogStream {
  using self = LogStream;
  using streamBuf = LogStreamBuffer<kSmallBufferSize>;

 public:
  LogStream(char* buf) : buffer_(buf) {}
  self& flush(void* output) {
    fwrite(buffer_.data(), buffer_.size(), 1, (FILE*)output);
    return *this;
  }
  template <typename T>
  void formatInteger(T v) {
    if (buffer_.avail() > kMaxNumericSize)
      buffer_.appendfx([v](char* dst, size_t avail) -> size_t {
        return itoa(dst, avail, v);
      });
  }

  self& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }
  self& operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
  }

  self& operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
  }

  template <typename T,
            typename = std::enable_if_t<std::is_integral_v<T>, void>>
  self& operator<<(T v) {
    static_assert(std::is_integral_v<T>,
                  "integer type required to instantiate this template");
    formatInteger(v);
    return *this;
  }

  self& operator<<(const void* ptr) {
    if (buffer_.avail() > sizeof(ptr))
      buffer_.appendfx([ptr](char* dst, size_t avail) -> size_t {
        return ptrtoa(dst, avail, ptr);
      });
    return *this;
  }

  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double v) {
    if (buffer_.avail() > kMaxNumericSize)
      buffer_.appendfx([v](char* dst, size_t avail) -> size_t {
        return dtoa_grisu2(dst, avail, v);
      });
    return *this;
  }
  // self& operator<<(long double);

  self& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  self& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const std::string_view& v) {
    buffer_.append(v.data(), v.size());
    return *this;
  }

  self& operator<<(const tm* t) {
    buffer_.appendfx([t](char* dst, size_t avail) -> size_t {
      return strftime(dst, avail, " %F %T ", t);
    });
    return *this;
  }

 private:
  streamBuf buffer_;
};

// LogMessage is an instance represents a log entry
template <Level LEVEL>
class LogMessage {
 public:
  LogMessage(const char* file, int line)
      : logstream_(gLogBuf<kSmallBufferSize, LEVEL>) {
    LogLv<LEVEL>::output = (LogLv<LEVEL>::output == nullptr)
                               ? LogLv<GLOBAL>::output
                               : LogLv<LEVEL>::output;
                               using fClkInMilliSec = FastClock<std::chrono::milliseconds>;
    auto ts =
        Singleton<fClkInMilliSec>::getInstance().Now();

    logstream_ << localtime(&ts) << file << ':' << line << ' '
               << LogLv<LEVEL>::id << ' ';
  }
  ~LogMessage() {
    logstream_ << '\n';
    logstream_.flush(LogLv<LEVEL>::output);
  }
  LogStream& stream() { return logstream_; };

 private:
  LogStream logstream_;
};
}  // namespace common
#endif  // TINYRPC_LOG_HPP
