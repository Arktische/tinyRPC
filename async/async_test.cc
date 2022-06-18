#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <future>
#include <random>
#include <stop_token>
#include <thread>

#include "async/io_context.hpp"
#include "awaitable.hpp"
#include "common/log.hpp"
#include "event.hpp"
#include "executor.hpp"
#include "generator.hpp"
#include "promise.hpp"
#include "sync_wait.hpp"
#include "task.hpp"
#include "task_container.hpp"
#include "thread_pool.hpp"
#include "when_all.hpp"

using namespace std::chrono_literals;

TEST(async_test, test_task) {
  async::thread_pool tp;
  auto bar = [&]() -> async::task<std::string> {
    co_await tp.schedule();
    std::this_thread::sleep_for(3s);
    co_return "bar";
  };

  auto foo = [&]() -> async::task<int> {
    auto str = co_await bar();
    std::cout << str << "\n";
    co_return 2022;
  };

  auto res = async::sync_wait(foo());
  std::cout << "result=" << res << "\n";
}

TEST(async_test, test_generator) {
  auto gen = []() -> async::generator<uint64_t> {
    uint64_t i = 0;
    while (true) {
      co_yield i++;
    }
  };

  for (auto val : gen()) {
    std::cout << val << ", ";

    if (val >= 5050) {
      break;
    }
  }
}

TEST(async_test, test_io_context) {
  async::io_context::kSize = 1024;
  async::io_context::kMaxEvent = 1024;
  async::io_context::kTimeout = 0;

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
      LOG(INFO)<<"new conn";
      int num_read = co_await ctx.recv(conn, buf, 1024);
      LOG(INFO)<<"read done";
      co_await ctx.send(conn, buf, num_read);
      LOG(INFO)<<"write done";
    }
  };

  std::jthread jt(server_task);
  jt.detach();
  async::sync_wait(ctx.run());
}