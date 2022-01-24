#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <common/non-copyable.h>
#include <common/log.hpp>
#include <string_view>
class EchoServer : common::NonCopyable{
public:
    EchoServer();
    EchoServer(int listen_fd);
    int bind(int listen_fd);
    int listen(std::string_view host, uint16_t port);
    int start();
private:
    int fd_;
};

#endif