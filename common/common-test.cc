//
// Created by tyx on 1/23/22.
//
#include "fast-clock.hpp"
#include "log.hpp"
#include <gtest/gtest.h>

TEST(log,testLogStream) {
  common::LogMessageStream<common::kSmallBufferSize> ms;
  ms<< __FILE__ << __LINE__ <<
  std::cout << ms.data();
  EXPECT_STREQ(ms.data().c_str(),"1.113411.23456");
}