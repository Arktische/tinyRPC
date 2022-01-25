//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_NON_COPYABLE_HPP
#define TINYRPC_NON_COPYABLE_HPP
namespace common {
class NonCopyable {
 public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};
}  // namespace common
#endif  // TINYRPC_NON_COPYABLE_HPP
