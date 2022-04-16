//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_NONCOPYABLE_HPP
#define TINYRPC_NONCOPYABLE_HPP
namespace common {
class NonCopyable {
 public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};
}  // namespace common
#endif  // TINYRPC_NONCOPYABLE_HPP
