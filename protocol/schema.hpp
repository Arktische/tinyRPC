#pragma once
#include <iostream>
#include <tuple>
#include <typeinfo>

template <typename T>
struct StructSchema {
  auto operator()() {
    return std::make_tuple();
  }
};

#define message(Struct, ...)                      \
  template <>                                  \
  StructSchema<Struct>() { \
    using T = Struct;                          \
    auto operator()() {\
return std::make_tuple(__VA_ARGS__);       \
    }\
}\

#define feild(field) \
std::make_tuple(#field, &T::field)

template <typename T, typename Fields, typename F, size_t... i>
inline constexpr void foreach (T&& obj, Fields && fields, F && f,
                               std::index_sequence<i...>) {
  auto _ = {(f(std::get<0>(std::get<i>(fields)),
               obj.*std::get<1>(std::get<i>(fields))),
             i)...,
            0ul};
}

template <typename T, typename F>
inline constexpr void foreach (T&& obj, F && f) {
  constexpr auto fields = StructSchema<std::decay_t<T>>();
  foreach (std::forward<T>(obj), fields, std::forward<F>(f),
           std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{})
    ;
}