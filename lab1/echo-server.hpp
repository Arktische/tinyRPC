#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <string_view>

#include <common/log.hpp>
#include <common/non-copyable.hpp>

using common::NonCopyable;

// EchoServer
class EchoServer : NonCopyable {
 public:
  static const int kMaxConn = 1024;
  EchoServer(std::string_view host, uint16_t port);
  explicit EchoServer(int listen_fd) : fd_(listen_fd) {}
  int start();

 private:
  void workerEventLoop();
  void listenerEventLoop();
  void on_read(int connfd);
  void on_write(int connfd);
  void on_peer_close(int connfd);
  int fd_;
  int epfd_;
  int rw_epfd_;

 private:
  alignas(128) std::atomic<bool> running_;
};

#endif