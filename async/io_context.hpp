#pragma once

#include <sys/epoll.h>
#include <sys/socket.h>

#include <coroutine>
#include <cstddef>
#include <stop_token>
#include <thread>
#include <utility>

#include "async/thread_pool.hpp"
#include "task.hpp"
namespace async {
template <typename T>
struct io_awaitable;

template <>
struct io_awaitable<epoll_event> : epoll_event {
  int epfd;
  auto await_ready() -> bool { return false; }
  auto await_ready() const noexcept -> bool { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    data.ptr = h.address();
    epoll_ctl(epfd, EPOLL_CTL_ADD, data.fd, this);
  }
  int await_resume() { return data.fd; }
};

class io_context {
 public:
  using type = io_context;
  using shared_type = std::shared_ptr<type>;
  static int kSize;
  static int kMaxEvent;
  static int kTimeout;

 public:
  io_context() { epfd_ = epoll_create(kSize); }
  ~io_context() = default;
  auto accept(int fd, sockaddr* addr, socklen_t* socklen) -> async::task<int> {
    co_await tp_.schedule();
    io_awaitable<epoll_event> new_conn_event{
        epoll_event{EPOLLIN, epoll_data_t{.fd = fd}}, epfd_};
    co_await new_conn_event;
    co_return ::accept(fd, addr, socklen);
  }

  auto send(int fd, char* buf, size_t len) -> async::task<size_t> {
    co_await tp_.schedule();
    io_awaitable<epoll_event> writeable_event{
        epoll_event{EPOLLOUT, epoll_data_t{.fd = fd}}, epfd_};
    co_await writeable_event;
    co_return ::send(fd, buf, len, 0);
  }

  auto recv(int fd, char* buf, size_t len) -> async::task<size_t> {
    co_await tp_.schedule();
    io_awaitable<epoll_event> readable_event{
        epoll_event{EPOLLIN, epoll_data_t{.fd = fd}}, epfd_};
    co_await readable_event;
    co_return ::recv(fd, buf, len, 0);
  }

  auto run() -> async::task<> {
    co_await tp_.schedule();
    auto epoll_task = [this]() {
      epoll_event ready_event[kMaxEvent];
      while (1) {
        int num_ready = epoll_wait(epfd_, ready_event, kMaxEvent, kTimeout);
        for (int i = 0; i < num_ready; ++i) {
          std::coroutine_handle<> h;
          h.from_address(ready_event[i].data.ptr);
          if (!h.done()) h.resume();
        }
      }
    };
    epoll_task();
  }

  void stop() {
    tp_.shutdown();
    close(epfd_);
  }

 private:
  int epfd_;
  thread_pool tp_;
};
}  // namespace async