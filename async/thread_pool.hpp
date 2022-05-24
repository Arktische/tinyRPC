#pragma once

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <ranges>
#include <thread>
#include <variant>
#include <vector>

#include "event.hpp"
#include "task.hpp"
namespace async {

template <class T, class V>
concept range_of =
    std::ranges::range<T> && std::is_same_v<V, std::ranges::range_value_t<T>>;

template <class T, class V>
concept sized_range_of = std::ranges::sized_range<T> &&
    std::is_same_v<V, std::ranges::range_value_t<T>>;

}  // namespace async::concepts
namespace async {

class thread_pool {
 public:
  class operation {
    friend class thread_pool;

    explicit operation(thread_pool& tp) noexcept;

   public:
    auto await_ready() noexcept -> bool { return false; }

    auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
        -> void;

    auto await_resume() noexcept -> void {}

   private:
    thread_pool& pool_;

    std::coroutine_handle<> await_co_{nullptr};
  };

  struct options {
    uint32_t thread_count = std::thread::hardware_concurrency();

    std::function<void(std::size_t)> on_thread_start_functor = nullptr;

    std::function<void(std::size_t)> on_thread_stop_functor = nullptr;
  };

  explicit thread_pool(options opts = options{
                           .thread_count = std::thread::hardware_concurrency(),
                           .on_thread_start_functor = nullptr,
                           .on_thread_stop_functor = nullptr});

  thread_pool(const thread_pool&) = delete;
  thread_pool(thread_pool&&) = delete;
  auto operator=(const thread_pool&) -> thread_pool& = delete;
  auto operator=(thread_pool&&) -> thread_pool& = delete;

  virtual ~thread_pool();

  auto thread_count() const noexcept -> uint32_t { return thread_.size(); }

    auto schedule() -> operation;

  template <typename functor, typename... arguments>
    auto schedule(functor&& f, arguments... args)
      -> task<decltype(f(std::forward<arguments>(args)...))> {
    co_await schedule();

    if constexpr (std::is_same_v<void, decltype(f(std::forward<arguments>(
                                           args)...))>) {
      f(std::forward<arguments>(args)...);
      co_return;
    } else {
      co_return f(std::forward<arguments>(args)...);
    }
  }

  auto resume(std::coroutine_handle<> handle) noexcept -> void;

  template <range_of<std::coroutine_handle<>> range_type>
  auto resume(const range_type& handles) noexcept -> void {
    size_.fetch_add(std::size(handles), std::memory_order::release);

    size_t null_handles{0};

    {
      std::scoped_lock lk{wait_mutex_};
      for (const auto& handle : handles) {
        if (handle != nullptr) [[likely]] {
          queue_.emplace_back(handle);
        } else {
          ++null_handles;
        }
      }
    }

    if (null_handles > 0) {
      size_.fetch_sub(null_handles, std::memory_order::release);
    }

    wait_cond_.notify_one();
  }

    auto yield() -> operation { return schedule(); }

  auto shutdown() noexcept -> void;

  auto size() const noexcept -> std::size_t {
    return size_.load(std::memory_order::acquire);
  }

  auto empty() const noexcept -> bool { return size() == 0; }

  auto queue_size() const noexcept -> std::size_t {
    std::atomic_thread_fence(std::memory_order::acquire);
    return queue_.size();
  }

  auto queue_empty() const noexcept -> bool { return queue_size() == 0; }

 private:
  options opt_;

  std::vector<std::jthread> thread_;

  std::mutex wait_mutex_;

  std::condition_variable_any wait_cond_;

  std::deque<std::coroutine_handle<>> queue_;

  auto executor(const std::stop_token& stop_token, std::size_t idx) -> void;

  auto schedule_impl(std::coroutine_handle<> handle) noexcept -> void;

  std::atomic<std::size_t> size_{0};

  std::atomic<bool> shutdown_{false};
};

}  // namespace async
