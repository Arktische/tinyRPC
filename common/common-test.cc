//
// Created by tyx on 1/23/22.
//
#include <gtest/gtest.h>

#include "fast-clock.hpp"
#include "log.hpp"

TEST(common, test_log) {
  using common::LogMessage;
  LOG(INFO) << "abvdsvdsavs";
}

TEST(common,test_convert) {
  char result[20];
  auto test_integer = -1122334455667788;
  size_t actual_size = common::convert<decltype(test_integer)>(result, 20, test_integer);
  std::cout << actual_size << '\n' << result << std::endl;
}

TEST(common, test_uint64div10) {
  char buf[32];
  auto value = -112233445566778899;
  uint64_t v = value>0?value:-value;
  char* p = buf;
  for (int i = 0; v != 0 && i < 31; i++) {
    __uint128_t vv = ((__uint128_t)v * 0xcccccccccccccccd)>> 67;
    int lsd = static_cast<int>(v - vv * 10);
    v = vv;
    *p++ = common::zero[lsd];
  }
  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  std::cout<<buf;
}