#pragma once

#include <atomic>
#include <coroutine>

#include "executor.hpp"

namespace async {
enum class resume_order_policy { lifo, fifo };

class event {
 public:
  struct awaiter {
    explicit awaiter(const event& e) noexcept : event_(e) {}

    [[nodiscard]] auto await_ready() const noexcept -> bool {
      return event_.is_set();
    }

    auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
        -> bool;

    auto await_resume() noexcept {}

    const event& event_;

    std::coroutine_handle<> awaiting_coroutine_;

    awaiter* next_{nullptr};
  };

  explicit event(bool initially_set = false) noexcept;
  ~event() = default;

  event(const event&) = delete;
  event(event&&) = delete;
  auto operator=(const event&) -> event& = delete;
  auto operator=(event&&) -> event& = delete;

  auto is_set() const noexcept -> bool {
    return state_.load(std::memory_order_acquire) == this;
  }

  auto set(resume_order_policy policy = resume_order_policy::lifo) noexcept
      -> void;

  template <concepts::executor executor_type>
  auto set(executor_type& e,
           resume_order_policy policy = resume_order_policy::lifo) noexcept
      -> void {
    void* old_value = state_.exchange(this, std::memory_order::acq_rel);
    if (old_value != this) {
      if (policy == resume_order_policy::fifo) {
        old_value = reverse(static_cast<awaiter*>(old_value));
      }

      auto* waiters = static_cast<awaiter*>(old_value);
      while (waiters != nullptr) {
        auto* next = waiters->next_;
        e.resume(waiters->awaiting_coroutine_);
        waiters = next;
      }
    }
  }

  auto operator co_await() const noexcept -> awaiter { return awaiter(*this); }

  auto reset() noexcept -> void;

 protected:
  friend struct awaiter;

  mutable std::atomic<void*> state_;

 private:
  auto reverse(awaiter* cur) -> awaiter*;
};

}  // namespace async
