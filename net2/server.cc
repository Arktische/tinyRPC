#include "server.hpp"

#include <common/log.hpp>
namespace net2 {
server::server(int fd,addr_type addr) :fd_(fd),addr_(addr), accept_ring_(new io_uring){
  auto ring = accept_ring_.get();
    io_uring_queue_init(1024, ring,0);
}
server::~server() {}
auto server::accept() ->async::generator<tcp_stream> {
  auto ring = accept_ring_.get();
  auto sqe = io_uring_get_sqe(ring);
  auto cqe = new io_uring_cqe;
  while (true) {
    sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe,fd_,addr_->saddr(),addr_->plen(),0);
    io_uring_submit(ring);
    io_uring_wait_cqe(ring,&cqe);
    co_yield tcp_stream(cqe->res);
  }
}
auto server::on(addr_type addr) -> server::return_type {
  errno = 0;
  auto fd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
  auto err = bind(fd,addr->saddr(),addr->len());

  if(fd < 0 || err != 0) {
    LOG(FATAL)<< strerror(errno);
  }

  return std::make_tuple(server(fd,addr), make_error_code(errno));
}
}  // namespace net2
