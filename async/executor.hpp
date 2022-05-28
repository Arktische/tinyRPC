#pragma once

#include <concepts>
#include <coroutine>

#include "awaitable.hpp"

namespace async {
template <typename type>
concept executor = requires(type t, std::coroutine_handle<> c) {
                     { t.schedule() } -> awaiter;
                     { t.yield() } -> awaiter;
                     { t.resume(c) } -> std::same_as<void>;
                   };

}  // namespace async
