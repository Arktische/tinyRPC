//
// Created by tyx on 1/20/22.
//
#include "echo-server.hpp"

EchoServer::EchoServer(int fd) {
  listen_fd_ = fd;
}

int EchoServer::listen(std::string_view host, uint16_t port) {
  return 0;
}

int EchoServer::bind(int listen_fd) {
  listen_fd_ = listen_fd;
}

