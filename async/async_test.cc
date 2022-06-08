#include <gtest/gtest.h>

#include <future>
#include <random>
#include <thread>

#include "awaitable.hpp"
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