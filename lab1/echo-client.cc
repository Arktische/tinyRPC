//
// Created by tyx on 2/4/22.
//
#include "echo-client.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <common/thread.hpp>
#include <common/log.hpp>
using common::Thread;


EchoClient::EchoClient(string_view host, uint16_t port): server_addr_({AF_INET, htons(port)}) {
  inet_pton(AF_INET,host.data(),&server_addr_.sin_addr.s_addr);
}



const char host[16]{"127.0.0.0.1"};
const uint16_t port{8888};


int main(int argc,char* argv[]) {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  if (fd == -1) {
    LOG(FATAL) << "create socket failed"
               << " errno: " << errno << " msg: " << strerror(errno);
    abort();
  }
  int err;
  struct sockaddr_in addr {
    AF_INET, htons(port)
  };
  inet_pton(AF_INET, host, &addr.sin_addr.s_addr);
}