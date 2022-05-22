#include "io.hpp"

namespace net2 {

io::io() : ring(new io_uring) { io_uring_queue_init(1024, ring, 0); }
io::~io() {
  io_uring_queue_exit(ring);
  delete ring;
}
}  // namespace net2