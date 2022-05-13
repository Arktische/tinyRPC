#include <gtest/gtest.h>

#include <algorithm>
#include <tuple>
#include <vector>

#include <common/tuple_util.hpp>
#include <protocol/archive.hpp>
#include <protocol/schema.hpp>
struct A {
  int a{2};
  char b{'c'};
  char* c{nullptr};
  std::vector<int> vec{1, 2, 3, 4, 5, 5, 6};
};

struct foo {
  char i0;
  unsigned int i1;
  unsigned short i2;
  unsigned long long i3;
  unsigned char ar[2];
  int q;
  std::size_t w;
  int* p1;
  const void* p2;
  int const** const**** p_crazy;
  const double d;
};

TEST(protocol_test, test_struct_schema) {
  foo f{'a', 11, 12, 13, {'b', 'c'}, 16, 17, 0, 0, 0, 30.0};
  std::cout << common::schema::get<11>(f);
}

TEST(protocol_test, test_archive) {
  codec::CodecArchive encoder(std::cout);
  foo f{'a', 11, 12, 13, {'b', 'c'}, 16, 17, 0, 0, 0, 30.0};
  encoder << common::schema::get<4>(f);
}

TEST(protocol_test, test_tuple_size) {
  A a;
  constexpr std::size_t size = common::schema::tuple_size<foo>();
}

message(foo, export(i0), export(i1), export(i2), export(i3),
        export(ar));
TEST(protocol_test, test_schema) {
  foo f{'a', 11, 12, 13, {'b', 'c'}, 16, 17, 0, 0, 0, 30.0};
  foo f2{'c', 999999, 12, 13, {'d', 'c'}, 16, 17, 0, 0, 0, 30.0};
  auto tp = std::make_tuple(f.i0, f.i1, f.i2, f.i3, f.ar);

  common::tuple::for_each(tp, [](auto& elem) { std::cout << elem<<','; });

  
  foreach (f, [](auto&& feildName, auto&& value) {
    std::cout << feildName << ":" << value << ",\n";
  });
}
