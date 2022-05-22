#include "listener.hpp"

#include <common/log.hpp>
namespace net2 {
listener::listener(int fd,addr_type addr) :fd_(fd),addr_(addr),ring_(new io_uring){
  io_uring_queue_init(1024,ring_,0);
}
listener::~listener() {}
auto listener::accept() ->async::generator<tcp_stream> {
  auto sqe = io_uring_get_sqe(ring_);
  auto cqe = new io_uring_cqe;
  while (true) {
    sqe = io_uring_get_sqe(ring_);
    io_uring_prep_accept(sqe,fd_,addr_->saddr(),addr_->plen(),0);
    io_uring_wait_cqe(ring_,&cqe);
    co_yield tcp_stream(cqe->res);
  }
}
auto listener::on(listener::addr_type addr) ->listener::return_type {
  auto fd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
  auto err = bind(fd,addr->saddr(),addr->len());

  if(fd < 0 || err != 0) {
    LOG(FATAL)<< strerror(errno);
  }

  return std::make_tuple(listener(fd,addr), make_error_code(errno));
}
}  // namespace net2
