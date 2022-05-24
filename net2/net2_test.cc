#include <gtest/gtest.h>

#include <async/thread_pool.hpp>
#include <common/log.hpp>

#include "address.hpp"
#include "server.hpp"
TEST(listener, basic) {
  auto addr = std::make_shared<net2::ipv4_address>("127.0.0.1", 1080);
  auto [listener, err] = net2::server::on(addr);
  if (err) {
    LOG(FATAL) << err.message();
  }
  async::thread_pool pool;

  auto after_accept_task = [&pool](net2::server& l) -> async::task<> {
    pool.schedule();
    for (auto c : l.accept()) {
    }
  };
}