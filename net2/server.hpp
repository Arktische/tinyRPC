#pragma once
#include <liburing.h>
#include <sys/epoll.h>

#include <stop_token>

#include <async/generator.hpp>
#include <async/thread_pool.hpp>
#include <common/error.hpp>

#include "address.hpp"
#include "tcp_stream.hpp"
namespace net2 {

template <typename T>
struct awaitable;

template <>
struct awaitable<epoll_event> : epoll_event {
  int epfd;
  auto await_ready() const noexcept -> bool { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    data.ptr = h.address();
    epoll_ctl(epfd, EPOLL_CTL_ADD, data.fd, this);
  }
  int await_resume() { return data.fd; }
};

inline auto make_await_epevent(uint32_t events, int fd, int epfd)
    -> awaitable<epoll_event> {
  return awaitable<epoll_event>{epoll_event{events, epoll_data_t{.fd = fd}},
                                epfd};
}

class server {
  using type = server;
  using return_type = std::tuple<server, std::error_code>;

 public:
  static auto listen(addr_type addr) -> return_type;
  ~server();

  auto accept() -> async::generator<tcp_stream>;

  auto run() -> async::task<>;

 private:
  server(int fd, addr_type addr);

 private:
  std::stop_token st_;
  int fd_;
  addr_type addr_;
  int epfd_;

  static int kMaxEvent;
  static int kTimeout;
};
}  // namespace net2