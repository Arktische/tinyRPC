//
// Created by tyx on 1/23/22.
//
#include <gtest/gtest.h>

#include <cinttypes>
#include <climits>
#include <random>

#include "fast-clock.hpp"
#include "log.hpp"
TEST(common, test_log) {
  using common::LogMessage;
  LOG(INFO) << "abvdsvdsavs";
}

TEST(common, test_convert_randomly) {
  char buf[32];
  char std[32];
  static const unsigned long kMaxTestCnt = 4096 * 4096;
  std::random_device rdev;
  std::mt19937_64 rng(rdev());
  std::uniform_int_distribution<std::mt19937_64::result_type> dist_ulong(
      0, ULONG_MAX);
  auto rand_test_cnt = dist_ulong(rng) % kMaxTestCnt;
  for (int i = 0; i < rand_test_cnt; ++i) {
    auto t = dist_ulong(rng);
    snprintf(std, 32, "%llu", t);
    common::convert(buf, 32, t);
    ASSERT_STREQ(buf, std);
  }

  for (int i = 0; i < rand_test_cnt; ++i) {
    auto t = dist_ulong(rng);
    snprintf(std, 32, "%ld", -t);
    common::convert(buf, 32, (long)-t);
    ASSERT_STREQ(buf, std);
  }
}

TEST(common, test_convert_single) {
  char buf[32];
  unsigned long t = ULONG_MAX;
  common::convert(buf, 32, t);
  std::cout << buf << ',' << t;
}

TEST(common, test_uint64div10) {
  char buf[32];
  auto value = -112233445566778899;
  uint64_t v = value > 0 ? value : -value;
  char* p = buf;
  for (int i = 0; v != 0 && i < 31; i++) {
    __uint128_t vv = ((__uint128_t)v * 0xcccccccccccccccd) >> 67;
    int lsd = static_cast<int>(v - vv * 10);
    v = vv;
    *p++ = common::zero[lsd];
  }
  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);
  std::cout << buf;
}

TEST(common, profile_convert) {
  char buf[32];
  common::convert(buf, 32, ULLONG_MAX);
}
