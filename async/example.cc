#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstring>

#include "async/sync_wait.hpp"
#include "common/log.hpp"
#include "io_context.hpp"
#include "task.hpp"
int async::io_context::kSize = 1024;
  int async::io_context::kMaxEvent = 1024;
  int async::io_context::kTimeout = 0;
int main() {
  async::io_context ctx;

  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  struct sockaddr_in addr {
    AF_INET, htons(1080)
  };
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

  if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) != 0) {
    LOG(FATAL) << strerror(errno);
  }

  char buf[1024];

  auto server_task = [&](std::stop_token st) -> async::task<> {
    sockaddr_in peer_addr;
    socklen_t len;

    while (!st.stop_requested()) {
      int conn = co_await ctx.accept(fd, (sockaddr*)&peer_addr, &len);
      LOG(INFO) << "new conn";
      int num_read = co_await ctx.recv(conn, buf, 1024);
      LOG(INFO) << "read done";
      co_await ctx.send(conn, buf, num_read);
      LOG(INFO) << "write done";
    }
  };

  std::jthread jt(server_task);
  jt.detach();
  ctx.run();
}