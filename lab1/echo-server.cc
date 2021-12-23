//
// Created by tyx on 12/23/21.
//

#include <echo-server.hpp>
int EchoServer::bind(int listen_fd) {
    fd_ = listen_fd;
}

int EchoServer::listen(std::string_view host, uint16_t port) {
    if((fd_ = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1) {

    }

}