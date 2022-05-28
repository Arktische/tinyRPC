#pragma once
#include <liburing.h>

#include <async/generator.hpp>
#include <async/thread_pool.hpp>
#include <common/error.hpp>

#include "address.hpp"
#include "io.hpp"
#include "tcp_stream.hpp"
namespace net2 {
class server {
  using type = server;
  using return_type = std::tuple<server, std::error_code>;

 public:
  static auto on(addr_type addr) -> return_type;
  ~server();

  auto accept() -> async::generator<tcp_stream>;

 private:
  server(int fd, addr_type addr);
  int fd_;
  addr_type addr_;
  io_handle_type accept_ring_;
};
}  // namespace net2