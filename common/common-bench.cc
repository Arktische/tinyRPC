//
// Created by tyx on 1/30/22.
//
#include <benchmark/benchmark.h>

#include <climits>
#include <random>
#include <cstdlib>
#include "dtoa-grisu2.hpp"
#include "fastclock.hpp"
#include "log.hpp"
static void bench_snprintf_itoa(benchmark::State& state) {
  char buf[32];
  auto base = 0xcccccccccccccccd;
  for (auto _ : state) {
    snprintf(buf, 32, "%lu", ++base);
  }
}

static void bench_tinyRPC_itoa(benchmark::State& state) {
  char buf[32];
  auto base = 0xcccccccccccccccd;
  for (auto _ : state) {
    common::convert(buf, 32, ++base);
  }
}

static void bench_tinyRPC_dtoa(benchmark::State& state) {
  char buf[64];
  auto base = 1.223344556677889900112233445566778899;
  for (auto _ : state) {
    common::dtoa_grisu2(buf, 48, base++);
  }
}

static void bench_snprintf_dtoa(benchmark::State& state) {
  char buf[64];
  auto base = 1.223344556677889900112233445566778899;
  for (auto _ : state) {
    snprintf(buf, 48, "%lf", base++);
  }
}

static void bench_tinyRPC_memcpy(benchmark::State& state) {
  char* dst_ = (char*)malloc(1024*1024*1024);
  char* src_ = (char*)malloc(1024*1024*1024);
  static const unsigned memsize = 1024*1024*1024;
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> distoffset(0,1024*1024*1024-1); // distribution in range [1, 6]
  std::uniform_int_distribution<std::mt19937::result_type> distsize(0,256);
  for (auto _ : state) {
    state.PauseTiming();
    auto doffset = distoffset(rng);
    auto soffset = distoffset(rng);
    auto size = distsize(rng);
    if(soffset + size >= memsize || doffset + size >= memsize|| std::abs(doffset-soffset)<=size)
    state.ResumeTiming();
    common::memcpy(dst_, src_, 1024);
  }
}

static void bench_std_memcpy(benchmark::State& state) {
  char dst[1024]{'0'};
  char src[1024]{'1'};
  char* dst_ = dst;
  char* src_ = src;
  for (auto _ : state) {
    std::memcpy(dst_, src_, 1024);
    std::swap(dst_, src_);
  }
}

static void bench_std_chrono_now(benchmark::State& state) {
  long s;
  for (auto _ : state) {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    s = now;
  }
}

static void bench_tinyRPC_fastclock_now(benchmark::State& state) {
  long s;
  for (auto _ : state) {
    auto now = common::Singleton<
                   common::FastClock<std::chrono::milliseconds>>::getInstance()
                   .Now();
    s = now;
  }
}

BENCHMARK(bench_snprintf_itoa);
BENCHMARK(bench_tinyRPC_itoa);
BENCHMARK(bench_snprintf_dtoa);
BENCHMARK(bench_tinyRPC_dtoa);
// BENCHMARK(bench_std_memcpy);
// BENCHMARK(bench_tinyRPC_memcpy);
BENCHMARK(bench_std_chrono_now);
BENCHMARK(bench_tinyRPC_fastclock_now);
BENCHMARK_MAIN();