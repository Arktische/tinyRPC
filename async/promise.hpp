#pragma once

#include <concepts>

#include "awaitable.hpp"

namespace async::concepts {

template <typename type, typename return_type>
concept promise =
    requires(type t) {
      { t.get_return_object() } -> std::convertible_to<std::coroutine_handle<>>;
      { t.initial_suspend() } -> awaiter;
      { t.final_suspend() } -> awaiter;
      { t.yield_value() } -> awaitable;
    } && requires(type t, return_type return_value) {
           std::same_as<decltype(t.return_void()), void> ||
               std::same_as<decltype(t.return_value(return_value)), void> ||
                 requires
           {
             t.yield_value(return_value);
           };
         };

}  // namespace async::concepts
