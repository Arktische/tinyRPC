//
// Created by tyx on 1/30/22.
//
#include <benchmark/benchmark.h>

#include <climits>

#include "log.hpp"
static void bench_snprintf(benchmark::State& state) {
  char buf[32];
  for (auto _ : state) {
    snprintf(buf, 32, "%lu", ULLONG_MAX);
  }
}

static void bench_tinyRPC_convert(benchmark::State& state) {
  char buf[32];
  for (auto _ : state) {
    common::convert(buf, 32, ULLONG_MAX);
  }
}

BENCHMARK(bench_snprintf);
BENCHMARK(bench_tinyRPC_convert);

BENCHMARK_MAIN();