#include "io.hpp"

namespace net2 {

io_context::io_context(int entries, uint32_t flags, uint32_t wq_fd) {
  io_uring_params p = {
      .flags = flags,
      .wq_fd = wq_fd,
  };
  io_uring_queue_init_params(entries, &ring, &p);
}

io_context::~io_context() noexcept { io_uring_queue_exit(&ring); }

auto io_context::read(int fd, void* buf, unsigned int nbytes, off_t offset,
                      uint8_t iflags) {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_read(sqe, fd, buf, nbytes, offset);
  return await_impl(sqe, iflags);
}

auto io_context::write(int fd, const void* buf, unsigned int nbytes,
                       off_t offset, uint8_t iflags) {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_write(sqe, fd, buf, nbytes, offset);
  return await_impl(sqe, iflags);
}

auto io_context::read_fixed(int fd, void* buf, unsigned int nbytes,
                            off_t offset, int buf_index,
                            uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_read_fixed(sqe, fd, buf, nbytes, offset, buf_index);
  return await_impl(sqe, iflags);
}

auto io_context::write_fixed(int fd, const void* buf, unsigned int nbytes,
                             off_t offset, int buf_index,
                             uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_write_fixed(sqe, fd, buf, nbytes, offset, buf_index);
  return await_impl(sqe, iflags);
}

auto io_context::recvmsg(int sockfd, msghdr* msg, uint32_t flags,
                         uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_recvmsg(sqe, sockfd, msg, flags);
  return await_impl(sqe, iflags);
}

auto io_context::sendmsg(int sockfd, const msghdr* msg, uint32_t flags,
                         uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_sendmsg(sqe, sockfd, msg, flags);
  return await_impl(sqe, iflags);
}

auto io_context::recv(int sockfd, void* buf, unsigned int nbytes,
                      uint32_t flags, uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_recv(sqe, sockfd, buf, nbytes, flags);
  return await_impl(sqe, iflags);
}

auto io_context::send(int sockfd, const void* buf, unsigned int nbytes,
                      uint32_t flags, uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_send(sqe, sockfd, buf, nbytes, flags);
  return await_impl(sqe, iflags);
}

auto io_context::poll(int fd, short poll_mask, uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_poll_add(sqe, fd, poll_mask);
  return await_impl(sqe, iflags);
}

auto io_context::yield(uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_nop(sqe);
  return await_impl(sqe, iflags);
}

auto io_context::accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags,
                        uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_accept(sqe, fd, addr, addrlen, flags);
  return await_impl(sqe, iflags);
}

auto io_context::connect(int fd, sockaddr* addr, socklen_t addrlen, int flags,
                         uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_connect(sqe, fd, addr, addrlen);
  return await_impl(sqe, iflags);
}

auto io_context::timeout(__kernel_timespec* ts, uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_timeout(sqe, ts, 0, 0);
  return await_impl(sqe, iflags);
}

auto io_context::close(int fd, uint8_t iflags) noexcept {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_close(sqe, fd);
  return await_impl(sqe, iflags);
}

auto io_context::shutdown(int fd, int how, uint8_t iflags) {
  auto* sqe = io_uring_get_sqe_safe();
  io_uring_prep_shutdown(sqe, fd, how);
  return await_impl(sqe, iflags);
}

auto io_context::await_impl(io_uring_sqe* sqe, uint8_t iflags) noexcept
    -> sqe_awaitable {
  io_uring_sqe_set_flags(sqe, iflags);
  return sqe_awaitable(sqe);
}

auto io_context::io_uring_get_sqe_safe() noexcept -> io_uring_sqe* {
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

}  // namespace net2
