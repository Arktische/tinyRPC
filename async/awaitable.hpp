#pragma once

#include <concepts>
#include <coroutine>
#include <type_traits>
#include <utility>

namespace async {

template <typename type>
concept awaiter =
    requires(type t, std::coroutine_handle<> c) {
      { t.await_ready() } -> std::same_as<bool>;
      std::same_as<decltype(t.await_suspend(c)), void> ||
          std::same_as<decltype(t.await_suspend(c)), bool> ||
          std::same_as<decltype(t.await_suspend(c)), std::coroutine_handle<>>;
      { t.await_resume() };
    };

template <typename type>
concept awaitable = requires(type t) {
                      { t.operator co_await() } -> awaiter;
                    };

template <typename type>
concept awaiter_void =
    requires(type t, std::coroutine_handle<> c) {
      { t.await_ready() } -> std::same_as<bool>;
      std::same_as<decltype(t.await_suspend(c)), void> ||
          std::same_as<decltype(t.await_suspend(c)), bool> ||
          std::same_as<decltype(t.await_suspend(c)), std::coroutine_handle<>>;
      { t.await_resume() } -> std::same_as<void>;
    };

template <typename type>
concept awaitable_void = requires(type t) {
                           { t.operator co_await() } -> awaiter_void;
                         };

template <awaitable awaitable, typename = void>
struct awaitable_traits {};

template <awaitable awaitable>
static auto get_awaiter(awaitable&& value) {
  return std::forward<awaitable>(value).operator co_await();
}

template <awaitable awaitable>
struct awaitable_traits<awaitable> {
  using awaiter_type = decltype(get_awaiter(std::declval<awaitable>()));
  using awaiter_return_type =
      decltype(std::declval<awaiter_type>().await_resume());
};

}  // namespace async
