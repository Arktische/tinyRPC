#pragma once

#include <cstddef>
#include <memory>

#include <async/task.hpp>
#include "address.hpp"

namespace net2 {

class tcp_stream {
 public:
  tcp_stream(int fd);
  tcp_stream(int fd, addr_type peer);
  ~tcp_stream();

  // asynchronize and non-blocking API
  auto read(char* dst_buf, std::size_t len) -> async::task<std::size_t>;
  auto write(char* src_buf, std::size_t len) -> async::task<std::size_t>;

 private:
 addr_type peer_addr_;
 int fd_;
};
}  // namespace net2