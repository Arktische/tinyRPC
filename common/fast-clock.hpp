//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_FAST_CLOCK_HPP
#define TINYRPC_FAST_CLOCK_HPP
#include "cacheline-padding.hpp"
#include "thread.hpp"
#include <atomic>
#include <chrono>
#include <ctime>
#include <unistd.h>
namespace common {

// FastClock using intel `rdtsc` instruction to calculate time, and calibrate it
// periodly, see https://en.wikipedia.org/wiki/Time_Stamp_Counter for more
// detail
class FastClock {
private:
  std::chrono::system_clock::time_point CACHE_LINE_PADDING(startRealTime_);
  std::chrono::steady_clock::time_point CACHE_LINE_PADDING(startMonoTime_);
  uint64_t CACHE_LINE_PADDING(firstTick_);
  std::atomic<double> CACHE_LINE_PADDING(nsPerTick_);

private:
  FastClock() {
    startRealTime_ = std::chrono::system_clock::now();
    startMonoTime_ = std::chrono::steady_clock::now();
    firstTick_ = readTick();
    Thread calibrator("FastClock-calibrator", [this]() {
      while (true) {
        sleep(2);
        calibrate();
      }
    });
  }
  void calibrate() {
    auto duration = std::chrono::steady_clock::now() - startMonoTime_;
    uint64_t tickDiff = readTick() - firstTick_;

    nsPerTick_ =
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() /
        tickDiff;
  }
  static uint64_t readTick() {
    uint32_t eax, edx;
    __asm__ volatile("rdtsc" : "=a"(eax), "=d"(edx));
    return ((uint64_t)edx) << 32 | (uint64_t)eax;
  }
};
} // namespace common

#endif // TINYRPC_FAST_CLOCK_HPP
