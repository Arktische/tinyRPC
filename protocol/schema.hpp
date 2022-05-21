#pragma once
#include <bits/c++config.h>

#include <iostream>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "factory.hpp"
template <typename T>
inline constexpr auto StructMeta() {
  return std::make_tuple();
}

#define message(Struct, ...)                   \
  template <>                                  \
  inline constexpr auto StructMeta<Struct>() { \
    using T = Struct;                          \
    return std::make_tuple(__VA_ARGS__);       \
  }

#define export(field) std::make_tuple(#field, &T::field)

// only c++20 supports this implementation
#if __cplusplus > 201703L
template <typename T, typename F>
// Traverse each field of a POD-type(C-compatible memory layout) struct object
// with given traversing function
inline constexpr void for_each(T&& obj, F&& f) {
  constexpr auto meta = StructMeta<std::decay_t<T>>();
  using meta_type = decltype(meta);
  []<std::size_t... I>(T && obj, F && f, meta_type fields,
                       std::index_sequence<I...>) {
    (f(std::get<0>(std::get<I>(fields)), obj.*std::get<1>(std::get<I>(fields))),
     ...);
  }
  (std::forward<T>(obj), std::forward<F>(f), std::forward<meta_type>(meta),
   std::make_index_sequence<std::tuple_size_v<meta_type>>{});
}
#else
 template <typename T, typename Fields, typename F, size_t... Is>
 inline constexpr void for_each (T&& obj, Fields && fields, F && f,
                                std::index_sequence<Is...>) {
   auto _ = {(f(std::get<0>(std::get<Is>(fields)),
                obj.*std::get<1>(std::get<Is>(fields))),
              Is)...,
             0ul};
 }

 template <typename T, typename F>
 inline constexpr void for_each (T&& obj, F && f) {
   constexpr auto fields = StructMeta<std::decay_t<T>>();
   foreach (std::forward<T>(obj), fields, std::forward<F>(f),
            std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{})
     ;
 }
#endif