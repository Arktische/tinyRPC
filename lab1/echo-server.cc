//
// Created by tyx on 12/23/21.
//

#include "echo-server.hpp"

#include <arpa/inet.h>
#include <thread>
#include <common/log.hpp>
#include <common/thread.hpp>

EchoServer::EchoServer(std::string_view host, uint16_t port) : running_(false) {
  // specify the non-blocking socket
  fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (fd_ == -1) {
    LOG(FATAL) << "create socket failed";
    abort();
  }
  int err;
  struct sockaddr_in addr {
    AF_INET, htons(port)
  };
  inet_pton(AF_INET, host.data(), &addr.sin_addr.s_addr);

  err = bind(fd_, (struct sockaddr*)&addr, sizeof(struct sockaddr));
  if (err != 0) {
    LOG(FATAL) << "bind sockaddr failed";
    abort();
  }
}

int EchoServer::start() {
  int err;
  err = listen(fd_, 1024);
  if (err != 0) return err;

  epfd_ = epoll_create(1024);
  epoll_event ls_event{EPOLLIN | EPOLLET,epoll_data_t {.fd=fd_}};
  err = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &ls_event);
  if (err != 0) return err;

  rw_epfd_ = epoll_create(1024);
  running_ = true;
  common::Thread listener("listener_event_loop",&EchoServer::handle_conn, this);
  common::Thread worker("worker_event_loop",&EchoServer::handle_rw, this);
  return 0;
}

void EchoServer::handle_rw() {
  struct epoll_event ev[1024];
  int err;
  while (running_) {
    auto nready = epoll_wait(rw_epfd_,ev,1024,0);
    for(int i = 0; i < nready;++i) {
      switch (ev[i].events) {
        case EPOLLIN:
          break;
        case EPOLLIN | EPOLLRDHUP:

        case EPOLLOUT:
          break;
        default:
          break;
      }
    }
  }
}

void EchoServer::handle_conn() {
  struct epoll_event ev[1024];
  sockaddr_in cli_addr{};
  int addr_size = sizeof(sockaddr);
  int err;
  while (running_) {
    auto nready = epoll_wait(epfd_, ev, 1024, 0);
    for (int i = 0; i < nready; ++i) {
      auto conn_fd = accept(fd_, (sockaddr*)&cli_addr,
                            reinterpret_cast<socklen_t*>(&addr_size));
      if (conn_fd == -1) {
        LOG(ERROR) << "accept connection failed, client ip:"
                   << inet_ntoa(cli_addr.sin_addr)
                   << ";port: " << cli_addr.sin_port;
      }
      epoll_event rw_ev{EPOLLIN|EPOLLET,epoll_data_t{.fd = conn_fd}};
      err = epoll_ctl(rw_epfd_,EPOLL_CTL_ADD,conn_fd,&rw_ev);
      if(err != 0) {
        LOG(ERROR) << "register new connection fd to epoll failed client ip:"
                   << inet_ntoa(cli_addr.sin_addr)
                   << ";port: " << cli_addr.sin_port;
      }
    }
  }
}

int main() {
  EchoServer echo_server("127.0.0.1", 8888);
  int err = echo_server.start();
  if (err != 0) {
    LOG(ERROR) << "EchoServer start failed";
  }
}