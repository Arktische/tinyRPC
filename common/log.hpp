//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include "non-copyable.h"
#include "fast-clock.hpp"
#include "singleton.hpp"
#include <iostream>
#include <vector>
#include <iomanip>

#define LOG(level) LogMessage(__FILE__, __LINE__,level).stream()

enum LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };
std::vector<std::string> kLogLevelStr= {"[DEBUG]","[INFO]","[WARN]","[ERROR]","[FATAL]"};
namespace common {
// LogMessageData
using common::NonCopyable;
using std::ostream;
using std::size_t;
using std::streambuf;
using std::cout;
static const int kMaxNumericSize = 48;
static const int kSmallBufferSize = 1024;
static const int kLargeBufferSize = 4096;
template <size_t SIZE> struct FixedBuffer {
  FixedBuffer() : cur_(data_) {}
  char *data() { return data_; };
  inline static int size() { return SIZE; }
  inline char *begin() { return data_; }
  inline char *end() { return data_ + SIZE; }

private:
  char *cur_;
  char data_[SIZE+2]{0};
};

template <size_t SIZE> class LogStreamBuf : public streambuf {
public:
  LogStreamBuf() { setp(buffer_.begin(), buffer_.end()); }
  virtual int_type overflow(int_type ch) { return ch; }

  inline char* data() {return buffer_.data();}
private:
  FixedBuffer<SIZE> buffer_;
};

template <size_t SIZE> class LogStream : public ostream {
  using SelfPtr = LogStream<SIZE> *;
public:
  LogStream() : ostream(&streambuf_) ,self_(this) {}
  char *str() { return streambuf_.data(); }

private:
  LogStreamBuf<SIZE> streambuf_;
  SelfPtr self_;
};


class LogMessage {
public:
  LogMessage(const char *file, int line, LogLevel lv): basename_(file),line_(line),level_(lv) {
    ts_ = common::Singleton<FastClock<std::chrono::milliseconds>>::getInstance().Now();
    logstream_ <<std::put_time(std::localtime(&ts_),"%F %T")
             <<basename_
             <<':'
             <<line_
             <<kLogLevelStr[level_];
  }
  ~LogMessage() {
    cout << logstream_.str();
  }
  ostream &stream() { return logstream_; };
private:
  LogStream<kSmallBufferSize> logstream_;
  LogLevel level_;
  const char* basename_;
  int line_;
  std::time_t ts_;
};
} // namespace common
#endif // TINYRPC_LOG_HPP
