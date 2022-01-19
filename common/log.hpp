//
// Created by tyx on 12/23/21.
//
#pragma once
#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include <iostream>
#define LOG(level) LogMessage<level>(__FILE__, __LINE__).stream()

namespace common {
// LogMessageData
template<class LEVEL_T>
struct LogMessageData {
  using self = LogMessageData;
  self operator<<(const char *);
};

template <class LEVEL_T> class LogMessage {
  LEVEL_T data;
  std::ostream ostream_;
  LogMessage(const char *file, int line);
  std::ostream &stream() { return ostream_; };
};
}
#endif // TINYRPC_LOG_HPP
