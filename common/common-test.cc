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