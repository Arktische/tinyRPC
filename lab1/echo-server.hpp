#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <netinet/in.h>
#ifdef __linux__
#include <sys/epoll.h>
# elif defined(__APPLE__)
#include <>
#endif
#include <sys/socket.h>

#include <string_view>

#include <common/log.hpp>
#include <common/non-copyable.hpp>
class EchoServer : common::NonCopyable {
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