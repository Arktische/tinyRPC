#include "tcp_stream.hpp"

namespace net2 {
tcp_stream::tcp_stream(int fd) {

}
tcp_stream::~tcp_stream() {}
auto tcp_stream::read(char* dst_buf, std::size_t len)->async::task<std::size_t> {

}
auto tcp_stream::write(char* src_buf, std::size_t len) ->async::task<std::size_t> {
  return async::task<std::size_t>();
}
tcp_stream::tcp_stream(int fd, tcp_stream::io_handle_type r_handle,
                       tcp_stream::io_handle_type w_handle) :read_ring_(r_handle),write_ring_(w_handle){
}

}