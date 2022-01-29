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

//template <typename T>
//struct make_fast_div10<T,typename std::enable_if<sizeof(T)<=4,uint32_t>::type> {
//    static const uint32_t base = 0xcccccccd;
//    static const int shift_offset = 32 + 3;
//    typedef uint64_t mul_type;
//};
//
//template <typename T>
//struct make_fast_div10<T, typename std::enable_if<true,uint64_t>::type> {
//    static const uint64_t base = 0xcccccccccccccccd;
//    static const int shift_offset = 64 + 3;
//    typedef __uint128_t mul_type;
//};
//
//template <typename T>


//template <typename T>
//struct make_fast_div10 {
//  template <T, typename en = void >
//  struct def<T,std::enable_if<true,T>> {
//
//  };
//};
//
//template <>
//struct make_fast_div10 {
//  static const uint32_t base = 0xcccccccd;
//  static const int shift_offset = 32 + 3;
//  typedef uint64_t mul_type;
//};
//
//template <typename T>
//struct make_fast_div10<
//    T, typename std::enable_if<(sizeof(T) > 4) && std::is_unsigned<T>::value,
//                               T>::type> {
//  static const uint64_t base = 0xcccccccccccccccd;
//  static const int shift_offset = 64 + 3;
//  typedef __uint128_t mul_type;
//};
}  // namespace common
#endif  // TINYRPC_TRAIT_HPP
