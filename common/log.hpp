//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_LOG_HPP
#define TINYRPC_LOG_HPP
#include <iostream>
#define LOG(level) LogMessage<level>(__FILE__,__LINE__).stream()

// LogMessageData
struct LogMessageData {

};



template<class LEVEL_T>
class LogMessage {
    LEVEL_T data;
    std::ostream ostream_;
    LogMessage(const char* file, int line);
    std::ostream& stream() {
        return ostream_;
    };
};
#endif //TINYRPC_LOG_HPP
