#pragma once

#include <cstddef>
#include <async/task.hpp>
#include <liburing.h>
#include <memory>
namespace net2 {

class tcp_stream {
 public:
  using io_handle_type = std::shared_ptr<io_uring>;
  explicit tcp_stream(int fd);
  tcp_stream(int fd, io_handle_type r_handle, io_handle_type w_handle);
  ~tcp_stream();

  // asynchronize and non-blocking API
  auto read(char* dst_buf, std::size_t len)->async::task<std::size_t>;
  auto write(char* src_buf, std::size_t len)->async::task<std::size_t>;

 private:
  io_handle_type read_ring_;
  io_handle_type write_ring_;
};
}