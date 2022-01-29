//
// Created by tyx on 12/23/21.
//

#include <common/fast-clock.hpp>
#include <common/fsm.hpp>
#include <common/log.hpp>
#include <common/non-copyable.hpp>
#include <common/thread.hpp>
#include <echo-server.hpp>
int EchoServer::bind(int listen_fd) { fd_ = listen_fd; }

int EchoServer::listen(std::string_view host, uint16_t port) {
  if ((fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
  }
}