//
// Created by tyx on 12/23/21.
//

#include "echo-server.hpp"

#include <arpa/inet.h>
#include <errno.h>

#include <thread>

#include <common/log.hpp>
#include <common/thread.hpp>

EchoServer::EchoServer(std::string_view host, uint16_t port) : running_(false) {
  // specify the non-blocking socket
  fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (fd_ == -1) {
    LOG(FATAL) << "create socket failed"
               << " errno: " << errno << " msg: " << strerror(errno);
    abort();
  }
  int err;
  struct sockaddr_in addr {
    AF_INET, htons(port)
  };
  inet_pton(AF_INET, host.data(), &addr.sin_addr.s_addr);

  err = bind(fd_, (struct sockaddr*)&addr, sizeof(struct sockaddr));
  if (err != 0) {
    LOG(FATAL) << "bind sockaddr failed"
               << " errno: " << errno << " msg: " << strerror(errno);
    abort();
  }
}

int EchoServer::start() {
  int err;
  err = listen(fd_, 1024);
  if (err != 0) {
    LOG(ERROR) << " errno: " << errno << " msg: " << strerror(errno);
    return err;
  }

  epfd_ = epoll_create(1024);
  // LT mode to avoid loop accept
  epoll_event ls_event{EPOLLIN, epoll_data_t{.fd = fd_}};
  err = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd_, &ls_event);
  if (err != 0) return err;

  rw_epfd_ = epoll_create(1024);
  running_ = true;
  common::Thread listener("listener_event_loop", &EchoServer::listenerEventLoop,
                          this);
  common::Thread worker("worker_event_loop", &EchoServer::workerEventLoop,
                        this);
  listener.detach();
  worker.join();
  return 0;
}

void EchoServer::workerEventLoop() {
  struct epoll_event ev[1024];
  int err;
  while (running_) {
    auto nready = epoll_wait(rw_epfd_, ev, 1024, 0);
    for (int i = 0; i < nready; ++i) {
      switch (ev[i].events) {
        case EPOLLRDHUP:  // peer close
          on_peer_close(ev[i].data.fd);
          break;
        case EPOLLIN:  // readable
          on_read(ev[i].data.fd);
          break;
        case EPOLLOUT:  // writeable
          on_write(ev[i].data.fd);
          break;
        default:
          break;
      }
    }
  }
}

void EchoServer::listenerEventLoop() {
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
      epoll_event rw_ev{EPOLLIN |EPOLLRDHUP| EPOLLET, epoll_data_t{.fd = conn_fd}};
      err = epoll_ctl(rw_epfd_, EPOLL_CTL_ADD, conn_fd, &rw_ev);
      if (err != 0) {
        LOG(ERROR) << "register new connection fd to epoll failed client ip:"
                   << inet_ntoa(cli_addr.sin_addr)
                   << ";port: " << cli_addr.sin_port;
      }
    }
  }
}

// on_peer_close
void EchoServer::on_peer_close(int connfd) {
  LOG(DEBUG)<<"void EchoServer::on_peer_close(int) called";
  int err = close(connfd);
  if (err != 0) {
    LOG(ERROR) << "close connfd failed"
               << " errno: " << errno << " msg: " << strerror(errno);
  }
}

// on_write
void EchoServer::on_write(int connfd) {
  LOG(DEBUG) << "void EchoServer::on_write(int) called";
}

// on_read
void EchoServer::on_read(int connfd) {
  char buf[1024];
  int nread = read(connfd,buf,1024);
  if(nread == EOF) {
    LOG(INFO) <<"peer close";
  }
  write(connfd,buf,nread);
  LOG(DEBUG) << "void EchoServer::on_read(int) called, data:\n"<<buf;
}

int main() {
  EchoServer echo_server("127.0.0.1", 8888);
  int err = echo_server.start();
  if (err != 0) {
    LOG(ERROR) << "EchoServer start failed";
  }
}