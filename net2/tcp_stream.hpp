#pragma once

#include <cstddef>
#include <async/task.hpp>
namespace net2 {

class tcp_stream {
 public:
  explicit tcp_stream(int fd);
  ~tcp_stream();

  // asynchronize and non-blocking API
  auto read(char* dst_buf, std::size_t len)->async::task<std::size_t>;
  auto write(char* src_buf, std::size_t len)->async::task<std::size_t>;
};
}