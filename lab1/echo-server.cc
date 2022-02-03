//
// Created by tyx on 12/23/21.
//

#include "echo-server.hpp"
#include <arpa/inet.h>
#include <common/log.hpp>
#include <common/thread.hpp>

EchoServer::EchoServer(std::string_view host, uint16_t port) {
  // specify the non-blocking socket
  fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if(fd_==-1) {
    LOG(FATAL)<<"create socket failed";abort();}
  int err;
  struct sockaddr_in addr{AF_INET, htons(port)};
  inet_pton(AF_INET,host.data(),&addr.sin_addr.s_addr);

  err = bind(fd_,(struct sockaddr*)&addr,sizeof(struct sockaddr));
  if(err != 0) {LOG(FATAL) << "bind sockaddr failed";abort();}
}

int EchoServer::start() {
  int err;
  err = listen(fd_,1024);
  if(!err) return err;
  epfd_ = epoll_create(1024);
  epoll_event event{EPOLLIN|EPOLLET};
  err = epoll_ctl(epfd_,EPOLL_CTL_ADD,fd_,&event);
  if(!err) return err;
  common::Thread ev_dispatcher("ev_dispatcher",[this](){
    struct epoll_event ev[1024];
    while (true) {
      auto nready = epoll_wait(epfd_,ev,1024,0);
      for(int i = 0; i < nready; ++i) {

      }
    }
  });
  return 0;
}

int main() {
  EchoServer echo_server("127.0.0.1",8888);
  int err = echo_server.start();
  if(err!=0) {
    LOG(ERROR)<< "listen local address failed";
  }
  echo_server.start();
}