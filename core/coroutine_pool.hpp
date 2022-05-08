#pragma once

#include <algorithm>
#include <atomic>
#include <concepts>
#include <coroutine>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>

#include "queue.hpp"
namespace core {
template <typename T>
struct future : std::future<T> {
  struct promise_type : std::promise<T> {
    std::future<T> get_return_object() { return this->get_future(); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };

  // Construct future object may after destruction of promise_type
  // So get furure before this time point
  explicit future(std::future<T>&& f) : std::future<T>(std::move(f)) {}
};

struct CoroutinePool {
  CoroutinePool(const CoroutinePool&) = delete;
  CoroutinePool operator=(const CoroutinePool&) = delete;
  // get implements singleton mode for CoroutinePool
  static CoroutinePool& get(int thread_number) {
    static CoroutinePool tp{thread_number};
    return tp;
  }

  template <typename T>
  struct awaitable {
    using promiseT = typename future<T>::promise_type;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<promiseT> h) {
      m_h = h;
      CoroutinePool::get(0).submit_coroutine(h);
    }
    std::coroutine_handle<promiseT> await_resume() { return m_h; }

    std::coroutine_handle<promiseT> m_h = nullptr;
  };

  template <std::invocable F>
  future<std::invoke_result_t<F>> submit(F task) {
    using resultT = std::invoke_result_t<F>;
    using promiseT = typename future<resultT>::promise_type;
    std::coroutine_handle<promiseT> h = co_await awaitable<resultT>();

    if constexpr (std::is_void_v<resultT>) {
      task();
    } else {
      h.promise().set_value(task());
    }
  }

  // coroutine_handle is also callable
  // So cannot overload submit simply
  void submit_coroutine(std::coroutine_handle<> h) { co_handles_.push(h); }

 private:
  explicit CoroutinePool(int thread_number) {
    for (int i = 0; i < thread_number; ++i) {
      joinalbe_t_.emplace(std::jthread([this] { this->worker(); }));
    }
  }

  ~CoroutinePool() {
    auto& q = co_handles_.destroy();
    while (!joinalbe_t_.empty()) {
      joinalbe_t_.pop();
    }
    // Add threads are shutdowned here

    while (!q.empty()) {
      q.front().destroy();
      q.pop();
    }
  }



  void worker() {
    while (auto task = co_handles_.pop()) {
      task.value().resume();
    }
  }

  ThreadSafeQueue<std::coroutine_handle<>> co_handles_;
  std::queue<std::jthread> joinalbe_t_;
};
}  // namespace core