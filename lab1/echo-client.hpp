//
// Created by tyx on 2/4/22.
//

#ifndef TINYRPC_ECHO_CLIENT_HPP
#define TINYRPC_ECHO_CLIENT_HPP
#include <string_view>
#include <sys/socket.h>
#include <netinet/in.h>
#include <common/non-copyable.hpp>
using common::NonCopyable;
using std::string_view;
class EchoClient : NonCopyable{
 static const int kMaxConn{1024};
 public:
  EchoClient(string_view host, uint16_t port);
 private:
  void on_write(int connfd);
  void on_read(int connfd);
  void on_peer_close(int connfd);
 private:
  sockaddr_in server_addr_;
  int epfd_;
};
#endif  // TINYRPC_ECHO_CLIENT_HPP
