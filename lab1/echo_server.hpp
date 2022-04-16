#ifndef ECHO_SERVER_HPP_
#define ECHO_SERVER_HPP_
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <bitset>
#include <string_view>
#include <vector>

#include <common/log.hpp>
#include <common/noncopyable.hpp>

using common::NonCopyable;
using std::pair;
using std::vector;
// EchoServer
class EchoServer : NonCopyable {
 public:
  static const int kMaxConn = 1024;
  EchoServer(std::string_view host, uint16_t port);
  explicit EchoServer(int listen_fd) : fd_(listen_fd) {}
  int start();

 private:
  void WorkerEventLoop();
  void ListenerEventLoop();
  void OnRead(int connfd);
  void OnWrite(int connfd);
  void OnPeerClose(int connfd);
  int fd_;
  int listenerEpfd_;
  int workerEpfd_;

 private:
  alignas(128) std::atomic<bool> running_;
};
#endif