#include "server.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <coroutine>

#include <common/log.hpp>

#include "async/sync_wait.hpp"
#include "async/task.hpp"
#include "common/error.hpp"
#include "net2/address.hpp"
#include "net2/tcp_stream.hpp"
namespace net2 {
server::server(int fd, addr_type addr) : fd_(fd), addr_(addr) {
  epfd_ = epoll_create(1024);
}
server::~server() {}
auto server::accept() -> async::generator<tcp_stream> {
  // RVO, no need to move
  auto event = make_await_epevent(EPOLLIN, fd_, epfd_);

  auto wait_on_newconn = [&]()->async::task<> {
    co_await event;
  };

  while (!st_.stop_requested()) {
    wait_on_newconn();
    auto peer_adder = ipv4_address();
    auto conn_fd = ::accept(fd_, peer_adder.saddr(), peer_adder.plen());
    co_yield tcp_stream(conn_fd);
  }
}
auto server::listen(addr_type addr) -> server::return_type {
  errno = 0;
  auto fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  auto err = bind(fd, addr->saddr(), addr->len());

  if (fd < 0 || err != 0) {
    LOG(FATAL) << strerror(errno);
  }

  return std::make_tuple(server(fd, addr), make_error_code(OSError(nil)));
}

auto server::run()->async::task<> {
  epoll_event ready_event[kMaxEvent];
  auto wait_on_epoll = [&]()-> async::task<int> {
    
  };
  while(!st_.stop_requested()) {
  

    int num_ready = epoll_wait(epfd_, ready_event, kMaxEvent, kTimeout);
    for(int i = 0; i < num_ready;++i) {
      std::coroutine_handle<> h;
      h.from_address(ready_event[i].data.ptr);
      if(!h.done()) h.resume();
    }
  }
  co_return;
}

}  // namespace net2
