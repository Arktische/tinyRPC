#include "server.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <coroutine>
#include <memory>
#include <stop_token>

#include <async/io_context.hpp>
#include <common/log.hpp>

#include "address.hpp"
#include "async/sync_wait.hpp"
#include "async/task.hpp"
#include "common/error.hpp"
#include "tcp_stream.hpp"
namespace net2 {
server::server(int fd, address::shared_type addr,
               async::io_context::shared_type io_ctx)
    : io_ctx_(io_ctx), fd_(fd), addr_(addr) {
      io_ctx_->run();
    }
server::~server() {}
auto server::accept(const std::stop_token& st) -> async::generator<tcp_stream> {
  ipv4_address addr;
  while(!st.stop_requested()) {
    int fd = co_await io_ctx_->accept(fd_, addr.saddr(), addr.plen());
    co_yield tcp_stream(fd,address::shared_type(&addr));
  }
}
auto server::listen(address::shared_type addr) -> server::return_type {
  errno = 0;
  auto fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  auto err = bind(fd, addr->saddr(), addr->len());

  if (fd < 0 || err != 0) {
    LOG(FATAL) << strerror(errno);
  }

  return std::make_tuple(
      server(fd, addr, std::make_shared<async::io_context>()),
      make_error_code(OSError(nil)));
}

}  // namespace net2
