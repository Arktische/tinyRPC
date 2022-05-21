#include "thread_pool.hpp"

#include <iostream>

namespace async {
thread_pool::operation::operation(thread_pool& tp) noexcept : pool_(tp) {}

auto thread_pool::operation::await_suspend(
    std::coroutine_handle<> awaiting_coroutine) noexcept -> void {
  await_co_ = awaiting_coroutine;
  pool_.schedule_impl(await_co_);
}

thread_pool::thread_pool(options opts) : opt_(std::move(opts)) {
  thread_.reserve(opt_.thread_count);

  for (uint32_t i = 0; i < opt_.thread_count; ++i) {
    thread_.emplace_back(
        [this, i](std::stop_token st) { executor(std::move(st), i); });
  }
}

thread_pool::~thread_pool() { shutdown(); }

auto thread_pool::schedule() -> operation {
  if (!shutdown_.load(std::memory_order::relaxed)) {
    size_.fetch_add(1, std::memory_order::release);
    return operation{*this};
  }

  throw std::runtime_error(
      "coro::thread_pool is shutting down, unable to schedule new tasks.");
}

auto thread_pool::resume(std::coroutine_handle<> handle) noexcept -> void {
  if (handle == nullptr) {
    return;
  }

  size_.fetch_add(1, std::memory_order::release);
  schedule_impl(handle);
}

auto thread_pool::shutdown() noexcept -> void {
  if (!shutdown_.exchange(true, std::memory_order::acq_rel)) {
    for (auto& thread : thread_) {
      thread.request_stop();
    }

    for (auto& thread : thread_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }
}

auto thread_pool::executor(const std::stop_token& stop_token, std::size_t idx)
    -> void {
  if (opt_.on_thread_start_functor != nullptr) {
    opt_.on_thread_start_functor(idx);
  }

  while (!stop_token.stop_requested()) {
    while (true) {
      std::unique_lock<std::mutex> lk{wait_mutex_};
      wait_cond_.wait(lk, stop_token, [this] { return !queue_.empty(); });
      if (queue_.empty()) {
        lk.unlock();

        break;
      }

      auto handle = queue_.front();
      queue_.pop_front();

      lk.unlock();

      handle.resume();
      size_.fetch_sub(1, std::memory_order::release);
    }
  }

  if (opt_.on_thread_stop_functor != nullptr) {
    opt_.on_thread_stop_functor(idx);
  }
}

auto thread_pool::schedule_impl(std::coroutine_handle<> handle) noexcept
    -> void {
  if (handle == nullptr) {
    return;
  }

  {
    std::scoped_lock lk{wait_mutex_};
    queue_.emplace_back(handle);
  }

  wait_cond_.notify_one();
}

}  // namespace async
