#pragma once

namespace common {
template <typename T>
concept is_context = requires(T v) {
  v.Value();
};
}  // namespace common