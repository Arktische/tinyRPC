#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <netinet/in.h>
#include <sys/socket.h>

#include <string_view>

#include <common/non-copyable.hpp>
class EchoServer : common::NonCopyable {
 public:
  EchoServer();
  EchoServer(int listen_fd);
  int bind(int listen_fd);
  int listen(std::string_view host, uint16_t port);
  int start();

 private:
  int listen_fd_;
};
#endif
