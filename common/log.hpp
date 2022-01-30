//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP

#include <cinttypes>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#include "dtoa-grisu2.hpp"
#include "fastclock.hpp"
#include "non-copyable.hpp"
#include "singleton.hpp"
#include "trait.hpp"

#define LOG(level) LogMessage(__FILE__, __LINE__, level).stream()

enum LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };
std::vector<std::string> kLogLevelStr = {"[DEBUG]", "[INFO]", "[WARN]",
                                         "[ERROR]", "[FATAL]"};
namespace common {
using std::cout;
using std::size_t;
using std::string;
static const int kSmallBufferSize = 1024;
static const int kLargeBufferSize = 4096;
static const int kMaxNumericSize = 48;
template <size_t SIZE>
class LogStreamBuffer : NonCopyable {
 public:
  LogStreamBuffer() : cur_(data_) {}
  char* data() { return data_; }
  inline static int size() { return SIZE; }
  inline static char* begin() { return data_; }
  inline static char* end() { return data_ + SIZE; }
  inline int avail() { return static_cast<int>(end() - cur_); }
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
  static thread_local char data_[64 + SIZE];
};

template <size_t SIZE>
thread_local char LogStreamBuffer<SIZE>::data_[64 + SIZE] = {0};
static const char digits[]{"9876543210123456789"};
static const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digitsHex[]{"0123456789ABCDEF"};
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

// Efficient Integer to String Conversions, by Matthew Wilson.
// refer to muduo, modified to support 64bit-integer
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>, void>>
size_t convert(char buf[], size_t maxlen, T value) {
  static_assert(std::is_integral<T>::value,
                "instantiated template param integerT requires integral type");
  using uiT = std::make_unsigned_t<T>;
  using umiT = make_umi_t<uiT>;
  static constexpr int shift_offset = sizeof(T) * 8 + 3;
  static constexpr auto base =
      static_cast<uiT>((sizeof(T) <= 4) ? 0xcccccccd : 0xcccccccccccccccd);

  uiT v = value > 0 ? value : -value;
  char* ptr = buf;
  int i;
  for (i = 0; v != 0 && 1 < maxlen - i; i++) {
    umiT q = (umiT)((umiT)v * base) >> shift_offset;
    *ptr++ = zero[static_cast<int>(v - q * 10)];
    v = q;
  }
  if (value < 0 && maxlen - i > 2) {
    *ptr++ = '-';
  }
  *ptr = '\0';
  std::reverse(buf, ptr);
  return ptr - buf;
}

template <typename T, typename = std::enable_if_t<std::is_pointer_v<T>, void>>
size_t convertHex(char buf[], size_t maxlen, T value) {
  auto v = reinterpret_cast<uintptr_t>(value);
  char* p = buf;
  int i;
  for (i = 0; v != 0 && 1 < maxlen - i; ++i) {
    auto q = v >> 4;
    *p++ = digitsHex[static_cast<int>(v - (q << 4))];
    v = q;
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

class LogStream {
  using selfPtr = LogStream*;
  using self = LogStream;
  using streamBuf = LogStreamBuffer<kSmallBufferSize>;

 public:
  LogStream() : self_(this) {}
  self& flush() { return *this; }
  template <typename T>
  void formatInteger(T v) {
    if (buffer_.avail() > kMaxNumericSize)
      buffer_.appendfx([v](char* dst, size_t avail) -> size_t {
        return convert(dst, avail, v);
      });
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
        return convertHex(dst, avail, ptr);
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
        return dtoa_milo(dst, avail, v);
      });
    return *this;
  }
  // self& operator<<(long double);

  self& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
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

 private:
  streamBuf buffer_;
  selfPtr self_;
};

class LogMessage {
 public:
  LogMessage(const char* file, int line, LogLevel lv) {
    logstream_ << file << ':' << line << ' ' << kLogLevelStr[lv];
  }
  ~LogMessage() { logstream_.flush(); }
  LogStream& stream() { return logstream_; };

 private:
  LogStream logstream_;
};
}  // namespace common
#endif  // TINYRPC_LOG_HPP
