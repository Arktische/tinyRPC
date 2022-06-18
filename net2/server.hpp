#pragma once
#include <liburing.h>
#include <sys/epoll.h>

#include <stop_token>

#include <async/generator.hpp>
#include <async/io_context.hpp>
#include <async/thread_pool.hpp>
#include <common/error.hpp>

#include "address.hpp"
#include "tcp_stream.hpp"
namespace net2 {

class server {
  using type = server;
  using return_type = std::tuple<server, std::error_code>;
  using shared_type = std::shared_ptr<type>;

 public:
  static auto listen(address::shared_type addr) -> return_type;
  ~server();

  auto accept(const std::stop_token& st) -> async::generator<tcp_stream>;

 private:
  server(int fd, address::shared_type addr,
         async::io_context::shared_type io_ctx);

 private:
  async::io_context::shared_type io_ctx_;
  int fd_;
  address::shared_type addr_;
};
}  // namespace net2