//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include "non-copyable.h"
#include <iostream>
#define LOG(level) LogMessage<level>(__FILE__, __LINE__).stream()

namespace common {
// LogMessageData
using common::NonCopyable;
using std::cout;
static const int kMaxNumericSize = 48;
static const int kSmallBufferSize = 1024;
static const int kLargeBufferSize = 4096;
// template <int SIZE> struct LogMessageStream : NonCopyable {
//   using self = LogMessageStream;
// public:
//   LogStream() { buffer_.reserve(SIZE);}
//   self operator<<(const char *s) {
//     if (s != nullptr)
//       buffer_.append(s);
//     else
//       buffer_.append("(null)");
//     return *this;
//   }
//   //  self& operator<<(short);
//   //  self& operator<<(unsigned short);
//     self& operator<<(int){
//       buffer_.append();
//     }
//   //  self& operator<<(unsigned int);
//   //  self& operator<<(long);
//   //  self& operator<<(unsigned long);
//   //  self& operator<<(long long);
//   //  self& operator<<(unsigned long long);
//
//   self &operator<<(bool v) {
//     buffer_.append(v ? "1" : "0");
//     return *this;
//   }
//   self &operator<<(float v) {
//     *this << static_cast<double>(v);
//     return *this;
//   }
//   // TODO: replace snprintf with Grisu3 algorithm
//   self &operator<<(double v) {
//
//#if __cplusplus <= 201703L
//     char nums[kMaxNumericSize];
//     std::snprintf(nums,kMaxNumericSize,"%.12g",v);
//     buffer_.append(nums);
//#elif __cplusplus > 201703L
//       // TODO
//#endif
//     return *this;
//   }
//   self &operator<<(char v) {
//     buffer_.append(1, v);
//     return *this;
//   }
//   self &operator<<(const std::string &v) {
//     buffer_.append(v);
//     return *this;
//   }
//   self &operator<<(const std::string_view &v) {
//     buffer_.append(v);
//     return *this;
//   }
//   const std::string& data() { return buffer_; }
//
// private:
//   std::string buffer_;
// };

template <int SIZE> struct FixedBuffer {
  FixedBuffer() : cur_(data_) {}
  char *data() { return data_; };
  inline static int size() { return SIZE; }

private:
  char *cur_;
  char data_[SIZE];
};

template <int SIZE> class LogStreamBuf : public std::streambuf {
public:
  LogStreamBuf() { setp(buffer_.data(), buffer_.size()); }
  virtual int_type overflow(int_type ch) { return ch; }

private:
  FixedBuffer<SIZE> buffer_
};

class LogMessage {
  std::ostream ostream_;
  LogMessage(const char *file, int line);
  std::ostream &stream() { return ostream_; };
};
} // namespace common
#endif // TINYRPC_LOG_HPP
