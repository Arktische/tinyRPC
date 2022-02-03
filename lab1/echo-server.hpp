#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <string_view>

#include <common/log.hpp>
#include <common/non-copyable.hpp>

// EchoServer
class EchoServer : common::NonCopyable {
 public:
  EchoServer(std::string_view host, uint16_t port);
  explicit EchoServer(int listen_fd):fd_(listen_fd){}
  int start();

 private:
  int fd_;
  int epfd_;
  int rw_epfd_;
};

#endif