#pragma once
#include "tcp_stream.hpp"
#include <common/error.hpp>
#include "address.hpp"
#include <liburing.h>
#include <async/generator.hpp>
namespace net2 {
class listener {
  using addr_type = std::shared_ptr<address>;
  using type = listener;
  using return_type = std::tuple<listener,std::error_code>;
 public:
  static auto on(addr_type addr)->return_type;
   ~listener();

  auto accept() ->async::generator<tcp_stream>;
 private:
  explicit listener(int fd,addr_type addr);
  int fd_;
  addr_type addr_;
  io_uring* ring_;
};
}