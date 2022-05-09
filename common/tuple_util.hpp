#pragma once
#include <tuple>
namespace common {

template <int...>
struct IndexTuple {};

// forward declare
template <int I, typename IndexTuple, typename... Types>
struct make_indexes_impl;

// declare
template <int I, int... Indexes, typename T, typename... Types>
struct make_indexes_impl<I, IndexTuple<Indexes...>, T, Types...> {
  using type = typename make_indexes_impl<I + 1, IndexTuple<Indexes..., I>,
                                          Types...>::type;
};

// terminate
template <int I, int... Indexes>
struct make_indexes_impl<I, IndexTuple<Indexes...>> {
  using type = IndexTuple<Indexes...>;
};

// type trait
template <typename... Types>
struct make_indexes : make_indexes_impl<0, IndexTuple<>, Types...> {};

template <int I, typename IndexTuple, typename... Types>
struct make_indexes_reverse_impl;

// declare
template <int I, int... Indexes, typename T, typename... Types>
struct make_indexes_reverse_impl<I, IndexTuple<Indexes...>, T, Types...> {
  using type =
      typename make_indexes_reverse_impl<I - 1, IndexTuple<Indexes..., I - 1>,
                                         Types...>::type;
};

// terminate
template <int I, int... Indexes>
struct make_indexes_reverse_impl<I, IndexTuple<Indexes...>> {
  using type = IndexTuple<Indexes...>;
};

template <typename... Types>
struct make_reverse_indexes
    : make_indexes_reverse_impl<sizeof...(Types), IndexTuple<>, Types...> {};

template <int N, int... Indexes>
struct make_indexes2 : make_indexes2<N - 1, N - 1, Indexes...> {};

template <int... Indexes>
struct make_indexes2<0, Indexes...> {
  typedef IndexTuple<Indexes...> type;
};

template <int end, int cur, int... Indexes>
struct make_indexes3 : make_indexes3<end, cur + 1, Indexes..., cur> {};

// when cur == end the list has been built.
template <int end, int... Indexes>
struct make_indexes3<end, end, Indexes...> {
  typedef IndexTuple<Indexes...> type;
};

namespace details {

template <typename Func, typename Last>
void for_each_impl(Func&& f, Last&& last) {
  f(std::forward<Last>(last));
}

template <typename Func, typename First, typename... Rest>
void for_each_impl(Func&& f, First&& first, Rest&&... rest) {
  f(std::forward<First>(first));
  for_each_impl(std::forward<Func>(f), rest...);
}

template <typename Func, int... Indexes, typename... Args>
void for_each_helper(Func&& f, IndexTuple<Indexes...>,
                     std::tuple<Args...>&& tup) {
  for_each_impl(std::forward<Func>(f),
                std::forward<Args>(std::get<Indexes>(tup))...);
}

}  // namespace details

template <typename Func, typename... Args>
void tp_for_each(Func&& f, std::tuple<Args...>& tup) {
  using namespace details;
  for_each_helper(forward<Func>(f), typename make_indexes<Args...>::type(),
                  std::tuple<Args...>(tup));
}

template <typename Func, typename... Args>
void tp_for_each(Func&& f, std::tuple<Args...>&& tup) {
  using namespace details;
  for_each_helper(forward<Func>(f), typename make_indexes<Args...>::type(),
                  forward<std::tuple<Args...>>(tup));
}

}  // namespace common