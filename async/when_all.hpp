#pragma once

#include <atomic>
#include <coroutine>
#include <ranges>
#include <tuple>
#include <vector>

#include "awaitable.hpp"

namespace async {
namespace detail {
struct void_value {};
class when_all_latch {
 public:
  explicit when_all_latch(std::size_t count) noexcept : count_(count + 1) {}

  when_all_latch(const when_all_latch&) = delete;
  when_all_latch(when_all_latch&& other) noexcept
      : count_(other.count_.load(std::memory_order::acquire)),
        awaiting_co_(
            std::exchange(other.awaiting_co_, nullptr)) {}

  auto operator=(const when_all_latch&) -> when_all_latch& = delete;
  auto operator=(when_all_latch&& other) noexcept -> when_all_latch& {
    if (std::addressof(other) != this) {
      count_.store(other.count_.load(std::memory_order::acquire),
                    std::memory_order::relaxed);
      awaiting_co_ = std::exchange(other.awaiting_co_, nullptr);
    }

    return *this;
  }

  auto is_ready() const noexcept -> bool {
    return awaiting_co_ != nullptr && awaiting_co_.done();
  }

  auto try_await(std::coroutine_handle<> awaiting_coroutine) noexcept -> bool {
    awaiting_co_ = awaiting_coroutine;
    return count_.fetch_sub(1, std::memory_order::acq_rel) > 1;
  }

  auto notify_awaitable_completed() noexcept -> void {
    if (count_.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      awaiting_co_.resume();
    }
  }

 private:
  std::atomic<std::size_t> count_;

  std::coroutine_handle<> awaiting_co_{nullptr};
};

template <typename task_container_type>
class when_all_ready_awaitable;

template <typename return_type>
class when_all_task;

template <>
class when_all_ready_awaitable<std::tuple<>> {
 public:
  constexpr when_all_ready_awaitable() noexcept = default;
  explicit constexpr when_all_ready_awaitable(std::tuple<>) noexcept {}

  constexpr auto await_ready() const noexcept -> bool { return true; }
  auto await_suspend(std::coroutine_handle<>) noexcept -> void {}
  auto await_resume() const noexcept -> std::tuple<> { return {}; }
};

template <typename... task_types>
class when_all_ready_awaitable<std::tuple<task_types...>> {
 public:
  explicit when_all_ready_awaitable(task_types&&... tasks) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<task_types>...>)
      : latch_(sizeof...(task_types)), task_(std::move(tasks)...) {}

  explicit when_all_ready_awaitable(std::tuple<task_types...>&& tasks) noexcept(
      std::is_nothrow_move_constructible_v<std::tuple<task_types...>>)
      : latch_(sizeof...(task_types)), task_(std::move(tasks)) {}

  when_all_ready_awaitable(const when_all_ready_awaitable&) = delete;
  when_all_ready_awaitable(when_all_ready_awaitable&& other) noexcept
      : latch_(std::move(other.latch_)), task_(std::move(other.task_)) {}

  auto operator=(const when_all_ready_awaitable&)
      -> when_all_ready_awaitable& = delete;
  auto operator=(when_all_ready_awaitable&&)
      -> when_all_ready_awaitable& = delete;

  auto operator co_await() & noexcept {
    struct awaiter {
      explicit awaiter(when_all_ready_awaitable& awaitable) noexcept
          : awaitable_(awaitable) {}

      auto await_ready() const noexcept -> bool {
        return awaitable_.is_ready();
      }

      auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
          -> bool {
        return awaitable_.try_await(awaiting_coroutine);
      }

      auto await_resume() noexcept -> std::tuple<task_types...>& {
        return awaitable_.task_;
      }

     private:
      when_all_ready_awaitable& awaitable_;
    };

    return awaiter{*this};
  }

  auto operator co_await() && noexcept {
    struct awaiter {
      explicit awaiter(when_all_ready_awaitable& awaitable) noexcept
          : awaitable_(awaitable) {}

      auto await_ready() const noexcept -> bool {
        return awaitable_.is_ready();
      }

      auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
          -> bool {
        return awaitable_.try_await(awaiting_coroutine);
      }

      auto await_resume() noexcept -> std::tuple<task_types...>&& {
        return std::move(awaitable_.task_);
      }

     private:
      when_all_ready_awaitable& awaitable_;
    };

    return awaiter{*this};
  }

 private:
  auto is_ready() const noexcept -> bool { return latch_.is_ready(); }

  auto try_await(std::coroutine_handle<> awaiting_coroutine) noexcept -> bool {
    std::apply([this](auto&&... tasks) { ((tasks.start(latch_)), ...); },
               task_);
    return latch_.try_await(awaiting_coroutine);
  }

  when_all_latch latch_;
  std::tuple<task_types...> task_;
};

template <typename task_container_type>
class when_all_ready_awaitable {
 public:
  explicit when_all_ready_awaitable(task_container_type&& tasks) noexcept
      : latch_(std::size(tasks)),
        task_(std::forward<task_container_type>(tasks)) {}

  when_all_ready_awaitable(const when_all_ready_awaitable&) = delete;
  when_all_ready_awaitable(when_all_ready_awaitable&& other) noexcept(
      std::is_nothrow_move_constructible_v<task_container_type>)
      : latch_(std::move(other.latch_)), task_(std::move(task_)) {}

  auto operator=(const when_all_ready_awaitable&)
      -> when_all_ready_awaitable& = delete;
  auto operator=(when_all_ready_awaitable&)
      -> when_all_ready_awaitable& = delete;

  auto operator co_await() & noexcept {
    struct awaiter {
      awaiter(when_all_ready_awaitable& awaitable) : awaitable_(awaitable) {}

      auto await_ready() const noexcept -> bool {
        return awaitable_.is_ready();
      }

      auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
          -> bool {
        return awaitable_.try_await(awaiting_coroutine);
      }

      auto await_resume() noexcept -> task_container_type& {
        return awaitable_.task_;
      }

     private:
      when_all_ready_awaitable& awaitable_;
    };

    return awaiter{*this};
  }

  auto operator co_await() && noexcept {
    struct awaiter {
      awaiter(when_all_ready_awaitable& awaitable) : awaitable_(awaitable) {}

      auto await_ready() const noexcept -> bool {
        return awaitable_.is_ready();
      }

      auto await_suspend(std::coroutine_handle<> awaiting_coroutine) noexcept
          -> bool {
        return awaitable_.try_await(awaiting_coroutine);
      }

      auto await_resume() noexcept -> task_container_type&& {
        return std::move(awaitable_.task_);
      }

     private:
      when_all_ready_awaitable& awaitable_;
    };

    return awaiter{*this};
  }

 private:
  auto is_ready() const noexcept -> bool { return latch_.is_ready(); }

  auto try_await(std::coroutine_handle<> awaiting_coroutine) noexcept -> bool {
    for (auto& task : task_) {
      task.start(latch_);
    }

    return latch_.try_await(awaiting_coroutine);
  }

  when_all_latch latch_;
  task_container_type task_;
};

template <typename return_type>
class when_all_task_promise {
 public:
  using coroutine_handle_type =
      std::coroutine_handle<when_all_task_promise<return_type>>;

  when_all_task_promise() noexcept {}

  auto get_return_object() noexcept {
    return coroutine_handle_type::from_promise(*this);
  }

  auto initial_suspend() noexcept -> std::suspend_always { return {}; }

  auto final_suspend() noexcept {
    struct completion_notifier {
      auto await_ready() const noexcept -> bool { return false; }
      auto await_suspend(coroutine_handle_type coroutine) const noexcept
          -> void {
        coroutine.promise().latch_->notify_awaitable_completed();
      }
      auto await_resume() const noexcept {}
    };

    return completion_notifier{};
  }

  auto unhandled_exception() noexcept { except_ptr = std::current_exception();
  }

  auto yield_value(return_type&& value) noexcept {
    return_value_ = std::addressof(value);
    return final_suspend();
  }

  auto start(when_all_latch& latch) noexcept -> void {
    latch_ = &latch;
    coroutine_handle_type::from_promise(*this).resume();
  }

  auto return_value() & -> return_type& {
    if (except_ptr) {
      std::rethrow_exception(except_ptr);
    }
    return *return_value_;
  }

  auto return_value() && -> return_type&& {
    if (except_ptr) {
      std::rethrow_exception(except_ptr);
    }
    return std::forward(*return_value_);
  }

 private:
  when_all_latch* latch_{nullptr};
  std::exception_ptr except_ptr;
  std::add_pointer_t<return_type> return_value_;
};

template <>
class when_all_task_promise<void> {
 public:
  using coroutine_handle_type =
      std::coroutine_handle<when_all_task_promise<void>>;

  when_all_task_promise() noexcept {}

  auto get_return_object() noexcept {
    return coroutine_handle_type::from_promise(*this);
  }

  auto initial_suspend() noexcept -> std::suspend_always { return {}; }

  auto final_suspend() noexcept {
    struct completion_notifier {
      auto await_ready() const noexcept -> bool { return false; }
      auto await_suspend(coroutine_handle_type coroutine) const noexcept
          -> void {
        coroutine.promise().latch_->notify_awaitable_completed();
      }
      auto await_resume() const noexcept -> void {}
    };

    return completion_notifier{};
  }

  auto unhandled_exception() noexcept -> void {
    except_ptr = std::current_exception();
  }

  auto return_void() noexcept -> void {}

  auto result() -> void {
    if (except_ptr) {
      std::rethrow_exception(except_ptr);
    }
  }

  auto start(when_all_latch& latch) -> void {
    latch_ = &latch;
    coroutine_handle_type::from_promise(*this).resume();
  }

 private:
  when_all_latch* latch_{nullptr};
  std::exception_ptr except_ptr;
};

template <typename return_type>
class when_all_task {
 public:
  template <typename task_container_type>
  friend class when_all_ready_awaitable;

  using promise_type = when_all_task_promise<return_type>;
  using coroutine_handle_type = typename promise_type::coroutine_handle_type;

  when_all_task(coroutine_handle_type coroutine) noexcept
      : coroutine_(coroutine) {}

  when_all_task(const when_all_task&) = delete;
  when_all_task(when_all_task&& other) noexcept
      : coroutine_(std::exchange(other.coroutine_, coroutine_handle_type{})) {
  }

  auto operator=(const when_all_task&) -> when_all_task& = delete;
  auto operator=(when_all_task&&) -> when_all_task& = delete;

  ~when_all_task() {
    if (coroutine_ != nullptr) {
      coroutine_.destroy();
    }
  }

  auto return_value() & -> decltype(auto) {
    if constexpr (std::is_void_v<return_type>) {
      coroutine_.promise().result();
      return void_value{};
    } else {
      return coroutine_.promise().return_value();
    }
  }

  auto return_value() const& -> decltype(auto) {
    if constexpr (std::is_void_v<return_type>) {
      coroutine_.promise().result();
      return void_value{};
    } else {
      return coroutine_.promise().return_value();
    }
  }

  auto return_value() && -> decltype(auto) {
    if constexpr (std::is_void_v<return_type>) {
      coroutine_.promise().result();
      return void_value{};
    } else {
      return coroutine_.promise().return_value();
    }
  }

 private:
  auto start(when_all_latch& latch) noexcept -> void {
    coroutine_.promise().start(latch);
  }

  coroutine_handle_type coroutine_;
};

template <concepts::awaitable awaitable,
          typename return_type = typename concepts::awaitable_traits<
              awaitable&&>::awaiter_return_type>
static auto make_when_all_task(awaitable a) -> when_all_task<return_type> {
  if constexpr (std::is_void_v<return_type>) {
    co_await static_cast<awaitable&&>(a);
    co_return;
  } else {
    co_yield co_await static_cast<awaitable&&>(a);
  }
}

}  // namespace detail

template <concepts::awaitable... awaitables_type>
[[nodiscard]] auto when_all(awaitables_type... awaitables) {
  return detail::when_all_ready_awaitable<
      std::tuple<detail::when_all_task<typename concepts::awaitable_traits<
          awaitables_type>::awaiter_return_type>...>>(
      std::make_tuple(detail::make_when_all_task(std::move(awaitables))...));
}

template <
    std::ranges::range range_type,
    concepts::awaitable awaitable_type = std::ranges::range_value_t<range_type>,
    typename return_type = typename concepts::awaitable_traits<
        awaitable_type>::awaiter_return_type>
[[nodiscard]] auto when_all(range_type awaitables)
    -> detail::when_all_ready_awaitable<
        std::vector<detail::when_all_task<return_type>>> {
  std::vector<detail::when_all_task<return_type>> output_tasks;

  if constexpr (std::ranges::sized_range<range_type>) {
    output_tasks.reserve(std::size(awaitables));
  }

  for (auto& a : awaitables) {
    output_tasks.emplace_back(detail::make_when_all_task(std::move(a)));
  }

  return detail::when_all_ready_awaitable(std::move(output_tasks));
}

}  // namespace async
