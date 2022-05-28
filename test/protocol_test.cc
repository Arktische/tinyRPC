#include <gtest/gtest.h>

#include <ostream>
#include <sstream>
#include <tuple>
#include <vector>

#include <protocol/archive.hpp>
#include <protocol/rpc.hpp>
#include <protocol/schema.hpp>

#include "protocol/factory.hpp"
struct A {
  char i0{'c'};
  unsigned int i1{1};
  unsigned short i2{2};
  unsigned long long i3{3};
  unsigned char ar[3]{'a', 'r', '\0'};
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

class B {
 public:
  int a{2};
  char b{'c'};
  std::vector<int> vec{1, 2, 3, 4, 5, 5, 6};
};

message(B, export(a), export(b), export(vec));
message(A, export(i0), export(i1), export(i2), export(i3), export(ar),
        export(vec));
std::ostream& operator<<(std::ostream& out, std::vector<int>& a) {
  out << '[';
  auto size = a.size();
  for (int i = 0; i < size - 1; ++i) {
    out << a[i] << ',';
  }
  if (size > 0) out << a[size - 1];
  out << ']';
  return out;
}

B GetB(A a) {
  std::cout << "GetB called\n";

  std::cout << "received request:{\n";
  for_each(a, [](auto&& feildName, auto&& value) {
    std::cout << '\t' << feildName << ":" << value << ",\n";
  });
  std::cout << "}\n";
  return B{3, 'd', {7, 8, 9, 10, 10, 11}};
}

rpc(A, B, GetB, codec::Binary<std::stringstream>);

TEST(protocol_test, dispatch_test) {
  A a;
  B b;
  auto ptr = dispatch("GetB", codec::Binary<std::stringstream>);
  std::stringstream ss;
  codec::Binary<std::stringstream> cdc(ss);
  for_each(a, [&cdc](auto&&, auto&& value) { cdc << value; });
  ptr->call(cdc);

  std::cout << "received response:{\n";
  for_each(b, [&cdc](auto&& field, auto&& value) {
    cdc >> value;
    std::cout << '\t' << field << ":" << value << ",\n";
  });
  std::cout << "}\n";
}