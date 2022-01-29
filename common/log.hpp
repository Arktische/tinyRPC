//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include <immintrin.h>

#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <vector>

#include "fast-clock.hpp"
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
#if defined(__x86_64) || defined(__x86_64__)
inline void memcpy(char* dst, const char* src, size_t size) {
  if (!(size >> 8)) {
    std::memcpy(dst, src, size);
    return;
  }
  auto p = reinterpret_cast<const char*>((reinterpret_cast<size_t>(src) + 31) &
                                         (~31ll));
  auto diff = p - src;
  std::memcpy(dst, src, diff);
  dst += diff;
  src += diff;
  size -= diff;
  size_t tms = size >> 5;
  while (tms--) {
    __m256i _src = _mm256_load_si256(reinterpret_cast<const __m256i*>(src));
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst), _src);
    dst += 16;
    src += 16;
  }
  std::memcpy(dst, src, size & 0x1f);
}
#endif

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
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

const static size_t kWordSize = sizeof(int*);

// Efficient Integer to String Conversions, by Matthew Wilson.
// refer to muduo
template <typename T>
size_t convert(char buf[], size_t len, T value) {
  static_assert(std::is_integral<T>::value,
                "instantiated template param integerT requires integral type");
  using uiT = std::make_unsigned_t<T>;
  using umiT = make_umi_t<uiT>;
  static constexpr int shift_offset = sizeof(T)*8+3;
  static constexpr auto base = static_cast<uiT>((sizeof(T) <= 4)?0xcccccccd:0xcccccccccccccccd);

  uiT v = value > 0 ? value : -value;
  char* p = buf;
  for (int i = 0; v != 0 && i < len; i++) {
    umiT vv = (umiT)((umiT)v * base)>> shift_offset;
    int lsd = static_cast<int>(v - vv * 10);
    v = vv;
    *p++ = zero[lsd];
  }
  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

size_t convertHex(char buf[], uintptr_t value) {
  uintptr_t i = value;
  char* p = buf;
  do {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);
  *p = '\0';
  std::reverse(buf, p);
  return p - buf;
}

/*
 Format a number with 5 characters, including SI units.
 [0,     999]
 [1.00k, 999k]
 [1.00M, 999M]
 [1.00G, 999G]
 [1.00T, 999T]
 [1.00P, 999P]
 [1.00E, inf)
*/
std::string formatSI(int64_t s) {
  auto n = static_cast<double>(s);
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%" PRId64, s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", n / 1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", n / 1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", n / 1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", n / 1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", n / 1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", n / 1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", n / 1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", n / 1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", n / 1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", n / 1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", n / 1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n / 1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", n / 1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", n / 1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", n / 1e15);
  else
    snprintf(buf, sizeof(buf), "%.2fE", n / 1e18);
  return buf;
}

/*
 [0, 1023]
 [1.00Ki, 9.99Ki]
 [10.0Ki, 99.9Ki]
 [ 100Ki, 1023Ki]
 [1.00Mi, 9.99Mi]
*/
std::string formatIEC(int64_t s) {
  auto n = static_cast<double>(s);
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;

  if (n < Ki)
    snprintf(buf, sizeof buf, "%" PRId64, s);
  else if (n < Ki * 9.995)
    snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
  else if (n < Ki * 99.95)
    snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
  else if (n < Ki * 1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

  else if (n < Mi * 9.995)
    snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
  else if (n < Mi * 99.95)
    snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
  else if (n < Mi * 1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

  else if (n < Gi * 9.995)
    snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
  else if (n < Gi * 99.95)
    snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
  else if (n < Gi * 1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

  else if (n < Ti * 9.995)
    snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
  else if (n < Ti * 99.95)
    snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
  else if (n < Ti * 1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

  else if (n < Pi * 9.995)
    snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
  else if (n < Pi * 99.95)
    snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
  else if (n < Pi * 1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

  else if (n < Ei * 9.995)
    snprintf(buf, sizeof buf, "%.2fEi", n / Ei);
  else
    snprintf(buf, sizeof buf, "%.1fEi", n / Ei);
  return buf;
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
    if (buffer_.avail() >= kMaxNumericSize) {
      buffer_.appendfx([&v](char* dst, size_t avail) -> size_t {
        return convert(dst, avail, v);
      });
    }
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
  self& operator<<(int v) {
    formatInteger(v);
    return *this;
  }
  self& operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
  }
  self& operator<<(long v) {
    formatInteger(v);
    return *this;
  }
  self& operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
  }
  self& operator<<(long long v) {
    formatInteger(v);
    return *this;
  }
  self& operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
  }

  self& operator<<(const void*);

  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
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
