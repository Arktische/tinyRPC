//
// Created by tyx on 4/16/22.
//

#ifndef TINYRPC_PROACTOR_HPP
#define TINYRPC_PROACTOR_HPP
#include <liburing.h>

#include <vector>

#include "common/thread.hpp"
#include "core/addr.h"
namespace net {
using common::Thread;
using std::vector;
class ProActor {
  static size_t kMaxQueueSize;
  ProActor() : ring_(new io_uring), accept_ring_(new io_uring) {
    io_uring_queue_init(kMaxQueueSize, ring_, 0);
    io_uring_queue_init(kMaxQueueSize, accept_ring_, 0);
  }

  void AddReadEvent() {}

  void AddWriteEvent() {}

  void AddAcceptEvent(int fd, NetAddress& addr) {
    auto sqe = io_uring_get_sqe(ring_);
    auto socklen = addr.getSockLen();
    io_uring_prep_accept(sqe, fd, addr.getSockAddr(), &socklen, 0);
    io_uring_sqe_set_data(sqe, nullptr);
    io_uring_submit(ring_);
  }

  void run() {}

 private:
  // every thread has a ring
  io_uring* ring_;
  io_uring* accept_ring_;
  pid_t tid_;
};
size_t ProActor::kMaxQueueSize = 16;
}  // namespace net
#endif  // TINYRPC_PROACTOR_HPP
