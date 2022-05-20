#pragma once

#include <concepts>
#include <coroutine>

#include "awaitable.hpp"

namespace async::concepts {
template <typename type>
concept executor = requires(type t, std::coroutine_handle<> c) {
  { t.schedule() } -> async::concepts::awaiter;
  { t.yield() } -> async::concepts::awaiter;
  { t.resume(c) } -> std::same_as<void>;
};

}  // namespace async::concepts
