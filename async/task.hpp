#pragma once

#include <coroutine>
#include <exception>
#include <utility>

#include "promise.hpp"

namespace async {
template <typename return_type = void>
class task;

namespace detail {
struct promise_base {
  friend struct final_awaitable;
  struct final_awaitable {
    auto await_ready() noexcept -> bool { return false; }

    template <typename promise_type>
    auto await_suspend(std::coroutine_handle<promise_type> coroutine) noexcept
        -> std::coroutine_handle<> {
      auto& promise = coroutine.promise();
      if (promise.continuation_ != nullptr) {
        return promise.continuation_;
      } else {
        return std::noop_coroutine();
      }
    }

    auto await_resume() noexcept -> void {}
  };

  promise_base() noexcept = default;
  ~promise_base() = default;

  auto initial_suspend() { return std::suspend_never{}; }

  auto final_suspend() noexcept(true) { return final_awaitable{}; }

  auto unhandled_exception() -> void { except_ptr_ = std::current_exception(); }

  auto continuation(std::coroutine_handle<> continuation) noexcept -> void {
    continuation_ = continuation;
  }

 protected:
  std::coroutine_handle<> continuation_{nullptr};
  std::exception_ptr except_ptr_{};
};

template <typename return_type>
struct promise final : public promise_base {
  using task_type = task<return_type>;
  using coroutine_handle = std::coroutine_handle<promise<return_type>>;

  promise() noexcept = default;
  ~promise() = default;

  auto get_return_object() noexcept -> task_type;

  auto return_value(return_type value) -> void {
    return_value_ = std::move(value);
  }

  auto result() const& -> const return_type& {
    if (except_ptr_) {
      std::rethrow_exception(except_ptr_);
    }

    return return_value_;
  }

  auto result() && -> return_type&& {
    if (except_ptr_) {
      std::rethrow_exception(except_ptr_);
    }

    return std::move(return_value_);
  }

 private:
  return_type return_value_;
};

template <>
struct promise<void> : public promise_base {
  using task_type = task<void>;
  using coroutine_handle = std::coroutine_handle<promise<void>>;

  promise() noexcept = default;
  ~promise() = default;

  auto get_return_object() noexcept -> task_type;

  auto return_void() noexcept -> void {}

  auto result() -> void {
    if (except_ptr_) {
      std::rethrow_exception(except_ptr_);
    }
  }
};

}  // namespace detail

template <typename return_type>
class task {
 public:
  using task_type = task<return_type>;
  using promise_type = detail::promise<return_type>;
  using co_handle_type = std::coroutine_handle<promise_type>;

  struct awaitable_base {
    auto await_ready() const noexcept -> bool {
      return !coroutine_ || coroutine_.done();
    }

    auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
        -> std::coroutine_handle<> {
      coroutine_.promise().continuation(awaiting_coroutine);
      return coroutine_;
    }

    co_handle_type coroutine_{nullptr};
  };

  task() noexcept : coroutine_(nullptr) {}

  task(co_handle_type handle) : coroutine_(handle) {}
  task(const task&) = delete;
  task(task&& other) noexcept
      : coroutine_(std::exchange(other.coroutine_, nullptr)) {}

  ~task() {
    if (coroutine_ != nullptr) {
      coroutine_.destroy();
    }
  }

  auto operator=(const task&) -> task& = delete;

  auto operator=(task&& other) noexcept -> task& {
    if (std::addressof(other) != this) {
      if (coroutine_ != nullptr) {
        coroutine_.destroy();
      }

      coroutine_ = std::exchange(other.coroutine_, nullptr);
    }

    return *this;
  }

  auto is_ready() const noexcept -> bool {
    return coroutine_ == nullptr || coroutine_.done();
  }

  auto done() const noexcept ->bool {
    return coroutine_.done();
  }

  auto resume() -> bool {
    if (!coroutine_.done()) {
      coroutine_.resume();
    }
    return !coroutine_.done();
  }

  auto destroy() -> bool {
    if (coroutine_ != nullptr) {
      coroutine_.destroy();
      coroutine_ = nullptr;
      return true;
    }

    return false;
  }

  auto operator co_await() const& noexcept {
    struct awaitable_lref : public awaitable_base {
      auto await_resume() -> decltype(auto) {
        if constexpr (std::is_same_v<void, return_type>) {
          this->coroutine_.promise().result();
          return;
        } else {
          return this->coroutine_.promise().result();
        }
      }
    };

    return awaitable_lref{coroutine_};
  }

  auto operator co_await() const&& noexcept {
    struct awaitable_rref : public awaitable_base {
      auto await_resume() -> decltype(auto) {
        if constexpr (std::is_same_v<void, return_type>) {
          this->coroutine_.promise().result();
          return;
        } else {
          return std::move(this->coroutine_.promise()).result();
        }
      }
    };

    return awaitable_rref{coroutine_};
  }

  auto promise() & -> promise_type& { return coroutine_.promise(); }

  auto promise() const& -> const promise_type& { return coroutine_.promise(); }
  auto promise() && -> promise_type&& {
    return std::move(coroutine_.promise());
  }

  auto handle() -> co_handle_type { return coroutine_; }

 private:
  co_handle_type coroutine_{nullptr};
};

namespace detail {
template <typename return_type>
inline auto promise<return_type>::get_return_object() noexcept
    -> task<return_type> {
  return task<return_type>{coroutine_handle::from_promise(*this)};
}

inline auto promise<void>::get_return_object() noexcept -> task<> {
  return task<>{coroutine_handle::from_promise(*this)};
}

}  // namespace detail

}  // namespace async
