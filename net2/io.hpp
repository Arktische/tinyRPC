#pragma once
#include <fcntl.h>
#include <liburing.h>  // http://git.kernel.dk/liburing
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <chrono>
#include <functional>
#include <system_error>

#include <async/task.hpp>

#include "async_sqe.hpp"
//#include "util.hpp"

namespace net2 {
class io_context {
 public:
  explicit io_context(int entries = 64, uint32_t flags = 0, uint32_t wq_fd = 0) {
    io_uring_params p = {
        .flags = flags,
        .wq_fd = wq_fd,
    };

    io_uring_queue_init_params(entries, &ring, &p);
  }

  ~io_context() noexcept { io_uring_queue_exit(&ring); }

  io_context(const io_context&) = delete;
  io_context& operator=(const io_context&) = delete;

 public:
  sqe_awaitable readv(int fd, const iovec* iovecs, unsigned nr_vecs,
                      off_t offset, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_readv(sqe, fd, iovecs, nr_vecs, offset);
    return await_work(sqe, iflags);
  }

  sqe_awaitable readv2(int fd, const iovec* iovecs, unsigned nr_vecs,
                       off_t offset, int flags, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_readv2(sqe, fd, iovecs, nr_vecs, offset, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable writev(int fd, const iovec* iovecs, unsigned nr_vecs,
                       off_t offset, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_writev(sqe, fd, iovecs, nr_vecs, offset);
    return await_work(sqe, iflags);
  }

  sqe_awaitable writev2(int fd, const iovec* iovecs, unsigned nr_vecs,
                        off_t offset, int flags, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_writev2(sqe, fd, iovecs, nr_vecs, offset, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable read(int fd, void* buf, unsigned nbytes, off_t offset,
                     uint8_t iflags = 0) {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_read(sqe, fd, buf, nbytes, offset);
    return await_work(sqe, iflags);
  }

  sqe_awaitable write(int fd, const void* buf, unsigned nbytes, off_t offset,
                      uint8_t iflags = 0) {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_write(sqe, fd, buf, nbytes, offset);
    return await_work(sqe, iflags);
  }

  sqe_awaitable read_fixed(int fd, void* buf, unsigned nbytes, off_t offset,
                           int buf_index, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_read_fixed(sqe, fd, buf, nbytes, offset, buf_index);
    return await_work(sqe, iflags);
  }

  sqe_awaitable write_fixed(int fd, const void* buf, unsigned nbytes,
                            off_t offset, int buf_index,
                            uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_write_fixed(sqe, fd, buf, nbytes, offset, buf_index);
    return await_work(sqe, iflags);
  }

  sqe_awaitable recvmsg(int sockfd, msghdr* msg, uint32_t flags,
                        uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_recvmsg(sqe, sockfd, msg, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable sendmsg(int sockfd, const msghdr* msg, uint32_t flags,
                        uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_sendmsg(sqe, sockfd, msg, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable recv(int sockfd, void* buf, unsigned nbytes, uint32_t flags,
                     uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_recv(sqe, sockfd, buf, nbytes, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable send(int sockfd, const void* buf, unsigned nbytes,
                     uint32_t flags, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_send(sqe, sockfd, buf, nbytes, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable poll(int fd, short poll_mask, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_poll_add(sqe, fd, poll_mask);
    return await_work(sqe, iflags);
  }

  sqe_awaitable yield(uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_nop(sqe);
    return await_work(sqe, iflags);
  }

  sqe_awaitable accept(int fd, sockaddr* addr, socklen_t* addrlen,
                       int flags = 0, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
    return await_work(sqe, iflags);
  }

  sqe_awaitable connect(int fd, sockaddr* addr, socklen_t addrlen,
                        int flags = 0, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_connect(sqe, fd, addr, addrlen);
    return await_work(sqe, iflags);
  }

  sqe_awaitable timeout(__kernel_timespec* ts, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_timeout(sqe, ts, 0, 0);
    return await_work(sqe, iflags);
  }

  sqe_awaitable close(int fd, uint8_t iflags = 0) noexcept {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_close(sqe, fd);
    return await_work(sqe, iflags);
  }

  sqe_awaitable shutdown(int fd, int how, uint8_t iflags = 0) {
    auto* sqe = io_uring_get_sqe_safe();
    io_uring_prep_shutdown(sqe, fd, how);
    return await_work(sqe, iflags);
  }

 private:
  static sqe_awaitable await_work(io_uring_sqe* sqe, uint8_t iflags) noexcept {
    io_uring_sqe_set_flags(sqe, iflags);
    return {sqe};
  }

 public:
  [[nodiscard]] io_uring_sqe* io_uring_get_sqe_safe() noexcept {
    auto* sqe = io_uring_get_sqe(&ring);
    if (__builtin_expect(sqe != nullptr, true)) {
      return sqe;
    } else {
      io_uring_cq_advance(&ring, cqe_count);
      cqe_count = 0;
      io_uring_submit(&ring);
      sqe = io_uring_get_sqe(&ring);
      if (__builtin_expect(sqe != nullptr, true)) return sqe;
    }
  }

  template <typename T, bool nothrow>
  T run(const async::task<T>& t) noexcept(nothrow) {
    while (!t.done()) {
      io_uring_submit_and_wait(&ring, 1);

      io_uring_cqe* cqe;
      unsigned head;

      io_uring_for_each_cqe(&ring, head, cqe) {
        ++cqe_count;
        auto coro = static_cast<resolver*>(io_uring_cqe_get_data(cqe));
        if (coro) coro->resolve(cqe->res);
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
