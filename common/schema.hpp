#pragma once
#include <iostream>
#include <tuple>
#include <typeinfo>

namespace common {
template <typename T>
inline constexpr auto StructMeta() {
  return std::make_tuple();
}

#define REFL(Struct, ...)                      \
  template <>                                  \
  inline constexpr auto StructMeta<Struct>() { \
    using T = Struct;                          \
    return std::make_tuple(__VA_ARGS__);       \
  };

#define FIELD(field) std::make_tuple(#field, &T::field)

template <typename T, typename Fields, typename F, size_t... Is>
inline constexpr void foreach (T&& obj, Fields && fields, F && f,
                               std::index_sequence<Is...>) {
  auto _ = {(f(std::get<0>(std::get<Is>(fields)),
               obj.*std::get<1>(std::get<Is>(fields))),
             Is)...,
            0ul};
}

template <typename T, typename F>
inline constexpr void foreach (T&& obj, F && f) {
  constexpr auto fields = StructMeta<std::decay_t<T>>();
  foreach (std::forward<T>(obj), fields, std::forward<F>(f),
           std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{})
    ;
}
}  // namespace common