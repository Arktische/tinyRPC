#include "event.hpp"

namespace async {
event::event(bool initially_set) noexcept
    : state_((initially_set) ? static_cast<void*>(this) : nullptr) {}

auto event::set(resume_order_policy policy) noexcept -> void {
  void* old_value = state_.exchange(this, std::memory_order::acq_rel);
  if (old_value != this) {
    if (policy == resume_order_policy::fifo) {
      old_value = reverse(static_cast<awaiter*>(old_value));
    }

    auto* waiters = static_cast<awaiter*>(old_value);
    while (waiters != nullptr) {
      auto* next = waiters->next_;
      waiters->awaiting_coroutine_.resume();
      waiters = next;
    }
  }
}

auto event::reverse(awaiter* cur) -> awaiter* {
  if (cur == nullptr || cur->next_ == nullptr) {
    return cur;
  }

  awaiter* prev = nullptr;
  awaiter* next = nullptr;
  while (cur != nullptr) {
    next = cur->next_;
    cur->next_ = prev;
    prev = cur;
    cur = next;
  }

  return prev;
}

auto event::awaiter::await_suspend(
    std::coroutine_handle<> awaiting_coroutine) noexcept -> bool {
  const void* const set_state = &event_;

  awaiting_coroutine_ = awaiting_coroutine;

  void* old_value = event_.state_.load(std::memory_order::acquire);
  do {
    if (old_value == set_state) {
      return false;
    }

    next_ = static_cast<awaiter*>(old_value);
  } while (!event_.state_.compare_exchange_weak(
      old_value, this, std::memory_order::release, std::memory_order::acquire));

  return true;
}

auto event::reset() noexcept -> void {
  void* old_value = this;
  state_.compare_exchange_strong(old_value, nullptr,
                                 std::memory_order::acquire);
}

}  // namespace async
