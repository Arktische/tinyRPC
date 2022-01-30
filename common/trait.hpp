//
// Created by tyx on 2022/1/27.
//

#ifndef TINYRPC_TRAIT_HPP
#define TINYRPC_TRAIT_HPP
#include <functional>
#include <tuple>

namespace common {
template <typename T>
inline constexpr auto StructSchema() {
  return std::make_tuple();
};

template <typename T>
struct function_trait;
template <typename R, typename... Args>
struct function_trait<std::function<R(Args...)>> {
  static const size_t nArgs = sizeof...(Args);
  typedef R res_type;
  template <size_t i>
  struct arg {
    typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
  };
};

template <typename T>
struct make_unsigned_mul_integer{
  typedef std::enable_if_t<std::is_unsigned_v<T>,uint64_t> type;
};

#if ULONG_MAX == ULONG_LONG_MAX
template<>
struct make_unsigned_mul_integer<unsigned long>{
  typedef __uint128_t type;
};
#else
template<>
struct make_unsigned_mul_integer<unsigned long>{
  typedef __uint64_t type;
};
#endif
template<>
struct make_unsigned_mul_integer<unsigned long long>{
  typedef __uint128_t type;
};

template<typename T> using make_umi_t =typename make_unsigned_mul_integer<T>::type;
}  // namespace common
#endif  // TINYRPC_TRAIT_HPP
