//
// Created by tyx on 2022/1/27.
//

#ifndef TINYRPC_TRAIT_HPP
#define TINYRPC_TRAIT_HPP
#include <functional>
#include <tuple>
#include <type_traits>

namespace common {
template <typename T, typename fxT>
struct has_member_fx;

#define HAS_MEMBER_FX(function_name)                   \
  template <typename T, typename R, typename... Arg>   \
  struct has_member_fx<T, std::function<R(Arg...)>> {  \
    typedef decltype(std::declval<T>().function_name(  \
        std::declval<Arg...>())) type;                 \
    static const bool value = std::is_same_v<R, type>; \
  };
struct AnyType {
  template <typename T>
  operator T();
};

#if __cplusplus >= 202002L
template <typename T>
consteval size_t CountMember(auto&&... Args) {
  if constexpr (!requires { T{Args...}; }) {  // (1)
    return sizeof...(Args) - 1;
  } else {
    return CountMember<T>(Args..., AnyType{});  // (2)
  }
}
#else
template <typename T, typename = void, typename... Ts>
struct CountMember {
  constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename... Ts>
struct CountMember<T, std::void_t<decltype(T{Ts{}...})>, Ts...> {
  constexpr static size_t value = CountMember<T, void, Ts..., AnyType>::value;
};

#endif
}  // namespace common
#endif  // TINYRPC_TRAIT_HPP
