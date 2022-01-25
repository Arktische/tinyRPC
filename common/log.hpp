//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include <immintrin.h>

#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "fast-clock.hpp"
#include "non-copyable.hpp"
#include "singleton.hpp"

#define LOG(level) LogMessage(__FILE__, __LINE__, level).stream()

enum LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };
std::vector<std::string> kLogLevelStr = {"[DEBUG]", "[INFO]", "[WARN]",
                                         "[ERROR]", "[FATAL]"};
namespace common {
using common::NonCopyable;
using std::cout;
using std::ostream;
using std::size_t;
using std::streambuf;
static const int kSmallBufferSize = 1024;
static const int kLargeBufferSize = 4096;

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
struct FixedBuffer {
  FixedBuffer() : cur_(data_) {}
  char* data() { return data_; };
  inline static int size() { return SIZE; }
  inline char* begin() { return data_; }
  inline char* end() { return data_ + SIZE; }
  inline int avail() { return static_cast<int>(end() - cur_); }
  void append(const char* buf, size_t len) {
    if (avail() > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

 private:
  char* cur_;
  char data_[SIZE + 2]{0};
};

template <size_t SIZE>
class LogStreamBuf : public streambuf {
 public:
  LogStreamBuf() {
    setp(buffer_.begin(), buffer_.end());
#if CMAKE_BUILD_TYPE == DEBUG
    cout << "Debug\n";
#endif
  }
  int_type overflow(int_type ch) override { return ch; }
  inline char* pbase() { return buffer_.data(); }

 private:
  FixedBuffer<SIZE> buffer_;
};

template <size_t SIZE>
class LogStream : public ostream {
  using SelfPtr = LogStream*;
  using Self = LogStream;

 public:
  LogStream() : ostream(&streambuf_), self_(this) {}
  inline char* str() { return streambuf_.pbase(); }
  Self& flush() { return *this; }

 private:
  LogStreamBuf<SIZE> streambuf_;
  SelfPtr self_;
};

class LogMessage {
 public:
  LogMessage(const char* file, int line, LogLevel lv) {
    auto ts_ =
        common::Singleton<FastClock<std::chrono::milliseconds>>::getInstance()
            .Now();
    logstream_ << std::put_time(std::localtime(&ts_), "%F %T ") << file << ':'
               << line << ' ' << kLogLevelStr[lv];
  }
  ~LogMessage() { cout << logstream_.str(); }
  ostream& stream() { return logstream_; };

 private:
  LogStream<kSmallBufferSize> logstream_;
};
}  // namespace common
#endif  // TINYRPC_LOG_HPP
