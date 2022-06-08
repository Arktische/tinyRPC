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
  static int kSize;
  static int kMaxEvent;
  static int kTimeout;

 public:
  io_context() { epfd_ = epoll_create(kSize); }
  ~io_context() = default;
  auto accept(int fd, sockaddr* addr, socklen_t* socklen) -> async::task<int> {
    tp_.schedule();
    io_awaitable<epoll_event> new_conn_event{
        epoll_event{EPOLLIN, epoll_data_t{.fd = fd}}, epfd_};
    co_await new_conn_event;
    co_return ::accept(fd, addr, socklen);
  }

  auto send(int fd, char* buf, size_t len) -> async::task<size_t> {
    tp_.schedule();
    io_awaitable<epoll_event> writeable_event{
        epoll_event{EPOLLOUT, epoll_data_t{.fd = fd}}, epfd_};
    co_await writeable_event;
    co_return ::send(fd, buf, len, 0);
  }

  auto recv(int fd, char* buf, size_t len) -> async::task<size_t> {
    tp_.schedule();
    io_awaitable<epoll_event> readable_event{
        epoll_event{EPOLLIN, epoll_data_t{.fd = fd}}, epfd_};
    co_await readable_event;
    co_return ::recv(fd, buf, len, 0);
  }

  void run() {
    auto epoll_task = [this](std::stop_token st) {
      epoll_event ready_event[kMaxEvent];
      while (!st.stop_requested()) {
        int num_ready = epoll_wait(epfd_, ready_event, kMaxEvent, kTimeout);
        for (int i = 0; i < num_ready; ++i) {
          std::coroutine_handle<> h;
          h.from_address(ready_event[i].data.ptr);
          if (!h.done()) h.resume();
        }
      }
    };
    std::jthread t(epoll_task);
    t.join();
    t_ = std::move(t);
  }

  void stop() {
    tp_.shutdown();
    t_.request_stop();
    close(epfd_);
  }

 private:
  int epfd_;
  thread_pool tp_;
  std::jthread t_;
};
}  // namespace async