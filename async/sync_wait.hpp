#pragma once

#include <condition_variable>
#include <mutex>

#include "awaitable.hpp"
#include "when_all.hpp"

namespace async {
namespace detail {
class sync_wait_event {
 public:
  sync_wait_event(bool initially_set = false) : set_(initially_set) {}
  sync_wait_event(const sync_wait_event&) = delete;
  sync_wait_event(sync_wait_event&&) = delete;
  auto operator=(const sync_wait_event&) -> sync_wait_event& = delete;
  auto operator=(sync_wait_event&&) -> sync_wait_event& = delete;
  ~sync_wait_event() = default;

  auto set() noexcept -> void {
    std::lock_guard<std::mutex> g{mutex_};
    set_ = true;
  }
  auto reset() noexcept -> void {
    std::lock_guard<std::mutex> g{mutex_};
    set_ = false;
  }
  auto wait() noexcept -> void {
    std::unique_lock<std::mutex> lk{mutex_};
    cond_.wait(lk, [this] { return set_; });
  }

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  bool set_{false};
};

class sync_wait_task_promise_base {
 public:
  sync_wait_task_promise_base() noexcept = default;
  virtual ~sync_wait_task_promise_base() = default;

  static auto initial_suspend() noexcept -> std::suspend_always { return {}; }

  auto unhandled_exception() -> void { except_ = std::current_exception(); }

 protected:
  sync_wait_event* event_{nullptr};
  std::exception_ptr except_;
};

template <typename return_type>
class sync_wait_task_promise : public sync_wait_task_promise_base {
 public:
  using coroutine_type =
      std::coroutine_handle<sync_wait_task_promise<return_type>>;

  sync_wait_task_promise() noexcept = default;
  ~sync_wait_task_promise() override = default;

  auto start(sync_wait_event& event) {
    event_ = &event;
    coroutine_type::from_promise(*this).resume();
  }

  auto get_return_object() noexcept {
    return coroutine_type::from_promise(*this);
  }

  auto yield_value(return_type&& value) noexcept {
    return_value_ = std::addressof(value);
    return final_suspend();
  }

  auto final_suspend() noexcept {
    struct completion_notifier {
      auto await_ready() const noexcept { return false; }
      auto await_suspend(coroutine_type coroutine) const noexcept {
        coroutine.promise().event_->set();
      }
      auto await_resume() noexcept {};
    };

    return completion_notifier{};
  }

  auto result() -> return_type&& {
    if (except_) {
      std::rethrow_exception(except_);
    }

    return static_cast<return_type&&>(*return_value_);
  }

 private:
  std::remove_reference_t<return_type>* return_value_;
};

template <>
class sync_wait_task_promise<void> : public sync_wait_task_promise_base {
  using coroutine_type = std::coroutine_handle<sync_wait_task_promise<void>>;

 public:
  sync_wait_task_promise() noexcept = default;
  ~sync_wait_task_promise() override = default;

  auto start(sync_wait_event& event) {
    event_ = &event;
    coroutine_type::from_promise(*this).resume();
  }

  auto get_return_object() noexcept {
    return coroutine_type::from_promise(*this);
  }

  auto final_suspend() noexcept {
    struct completion_notifier {
      auto await_ready() noexcept { return false; }
      auto await_suspend(coroutine_type coroutine) noexcept {
        coroutine.promise().event_->set();
      }
      auto await_resume() noexcept {};
    };

    return completion_notifier{};
  }

  auto return_void() noexcept -> void {}

  auto result() -> void {
    if (except_) {
      std::rethrow_exception(except_);
    }
  }
};

template <typename return_type>
class sync_wait_task {
 public:
  using promise_type = sync_wait_task_promise<return_type>;
  using coroutine_type = std::coroutine_handle<promise_type>;

  sync_wait_task(coroutine_type coroutine) noexcept : coroutine_(coroutine) {}

  sync_wait_task(const sync_wait_task&) = delete;
  sync_wait_task(sync_wait_task&& other) noexcept
      : coroutine_(std::exchange(other.coroutine_, coroutine_type{})) {}
  auto operator=(const sync_wait_task&) -> sync_wait_task& = delete;
  auto operator=(sync_wait_task&& other) noexcept -> sync_wait_task& {
    if (std::addressof(other) != this) {
      coroutine_ = std::exchange(other.coroutine_, coroutine_type{});
    }

    return *this;
  }

  ~sync_wait_task() {
    if (coroutine_) {
      coroutine_.destroy();
    }
  }

  auto start(sync_wait_event& event) noexcept {
    coroutine_.promise().start(event);
  }

  auto return_value() -> decltype(auto) {
    if constexpr (std::is_same_v<void, return_type>) {
      coroutine_.promise().result();
      return;
    } else {
      return coroutine_.promise().result();
    }
  }

 private:
  coroutine_type coroutine_;
};

template <awaitable awaitable_type,
          typename return_type =
              typename awaitable_traits<awaitable_type>::awaiter_return_type>
auto make_sync_wait_task(awaitable_type&& a) -> sync_wait_task<return_type> {
  if constexpr (std::is_void_v<return_type>) {
    co_await std::forward<awaitable_type>(a);
    co_return;
  } else {
    co_yield co_await std::forward<awaitable_type>(a);
  }
}

}  // namespace detail

template <awaitable awaitable_type>
auto sync_wait(awaitable_type&& a) -> decltype(auto) {
  detail::sync_wait_event e{};
  auto task = detail::make_sync_wait_task(std::forward<awaitable_type>(a));
  task.start(e);
  e.wait();

  return task.return_value();
}

}  // namespace async
