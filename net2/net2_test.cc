#include <gtest/gtest.h>
#include <cstdio>
#include <fcntl.h>
#include <async/thread_pool.hpp>
#include <common/log.hpp>

#include "address.hpp"
#include "io_context.hpp"

TEST(net2_test, test_io_context) {
  net2::io_context io_ctx;
  int stdout = open("test.txt",O_CREAT,0);
  char buf[1024]{
      "TEST(listener, basic) {\
  auto addr = std::make_shared<net2::ipv4_address>(\"127.0.0.1\", 1080);\
  auto [listener, err] = net2::server::on(addr);\
  if (err) {\
    LOG(FATAL) << err.message();\
  }\
  async::thread_pool pool;\
\
  auto after_accept_task = [&pool](net2::server& l) -> async::task<> {\
    pool.schedule();\
    for (auto c : l.accept()) {\
    }\
  };\
}"};
  auto task = [&]()->async::task<int> {
    std::cout <<"before write\n";
    co_await io_ctx.write(stdout, buf, 12, 0);
    co_return 0;
  };
  
  io_ctx.run(task());
}