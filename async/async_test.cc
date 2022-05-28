#include <gtest/gtest.h>

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
TEST(async_test, test_task) {
  auto square = [](uint64_t x) -> async::task<uint64_t> { co_return x* x; };
  auto output = async::sync_wait(square(2));
}