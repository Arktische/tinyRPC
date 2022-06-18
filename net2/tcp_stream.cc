#include "tcp_stream.hpp"
#include "net2/address.hpp"

namespace net2 {
tcp_stream::tcp_stream(int fd) {}
tcp_stream::~tcp_stream() {}
tcp_stream::tcp_stream(int fd, address::shared_type peer) :peer_addr_(peer) {
    
}


auto tcp_stream::read(char* dst_buf, std::size_t len)
    -> async::task<std::size_t> {}
auto tcp_stream::write(char* src_buf, std::size_t len)
    -> async::task<std::size_t> {
  return async::task<std::size_t>();
}

}  // namespace net2