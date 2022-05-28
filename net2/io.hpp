#pragma once
#include <fcntl.h>
#include <liburing.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <chrono>
#include <functional>
#include <memory>
#include <system_error>

#include <async/task.hpp>

#include "async_sqe.hpp"

namespace net2 {
using io_handle_type = std::shared_ptr<io_uring>;
class io_context {
 public:
  io_context(int entries = 64, uint32_t flags = 0, uint32_t wq_fd = 0);

  ~io_context() noexcept;

  io_context(const io_context&) = delete;
  io_context& operator=(const io_context&) = delete;

 public:
  auto read(int fd, void* buf, unsigned nbytes, off_t offset,
            uint8_t iflags = 0);

  auto write(int fd, const void* buf, unsigned nbytes, off_t offset,
             uint8_t iflags = 0);

  auto read_fixed(int fd, void* buf, unsigned nbytes, off_t offset,
                  int buf_index, uint8_t iflags = 0) noexcept;

  auto write_fixed(int fd, const void* buf, unsigned nbytes, off_t offset,
                   int buf_index, uint8_t iflags = 0) noexcept;

  auto recvmsg(int sockfd, msghdr* msg, uint32_t flags,
               uint8_t iflags = 0) noexcept;

  auto sendmsg(int sockfd, const msghdr* msg, uint32_t flags,
               uint8_t iflags = 0) noexcept;

  auto recv(int sockfd, void* buf, unsigned nbytes, uint32_t flags,
            uint8_t iflags = 0) noexcept;

  auto send(int sockfd, const void* buf, unsigned nbytes, uint32_t flags,
            uint8_t iflags = 0) noexcept;

  auto poll(int fd, short poll_mask, uint8_t iflags = 0) noexcept;

  auto yield(uint8_t iflags = 0) noexcept;

  auto accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags = 0,
              uint8_t iflags = 0) noexcept;

  auto connect(int fd, sockaddr* addr, socklen_t addrlen, int flags = 0,
               uint8_t iflags = 0) noexcept;

  auto timeout(__kernel_timespec* ts, uint8_t iflags = 0) noexcept;

  auto close(int fd, uint8_t iflags = 0) noexcept;

  auto shutdown(int fd, int how, uint8_t iflags = 0);

 private:
  static auto await_impl(io_uring_sqe* sqe, uint8_t iflags) noexcept
      -> sqe_awaitable;

 public:
  auto io_uring_get_sqe_safe() noexcept -> io_uring_sqe*;

  template <typename T, bool nothrow>
  T run(const async::task<T>& t) noexcept(nothrow) {
    while (!t.done()) {
      io_uring_submit_and_wait(&ring, 1);

      io_uring_cqe* cqe;
      unsigned head;

      io_uring_for_each_cqe(&ring, head, cqe) {
        ++cqe_count;
        auto co = static_cast<resolver*>(io_uring_cqe_get_data(cqe));
        if (co) co->resolve(cqe->res);
      }

      io_uring_cq_advance(&ring, cqe_count);
      cqe_count = 0;
    }

    return t.get_result();
  }

 private:
  io_uring ring{};
  unsigned cqe_count = 0;
};

}  // namespace net2
