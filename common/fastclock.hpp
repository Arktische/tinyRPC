//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_FASTCLOCK_HPP
#define TINYRPC_FASTCLOCK_HPP
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <ctime>

#include "align.hpp"
#include "thread.hpp"
namespace common {

// FastClock using intel `rdtsc` instruction to calculate time, and calibrate it
// periodly, see https://en.wikipedia.org/wiki/Time_Stamp_Counter for more
// detail.
using std::time_t;
using std::chrono::seconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::this_thread::sleep_for;
template <class AccuracyT>
class FastClock {
  static const size_t cache_align = hw_destructive_interference_size;

 public:
  time_t Now() {
    int64_t dur = (readTick() - firstTick_) * durationPerTick_;
    return system_clock::to_time_t(startRealTime_ + AccuracyT(dur));
  }

 private:
  alignas(cache_align) system_clock::time_point startRealTime_;
  alignas(cache_align) steady_clock::time_point startMonoTime_;
  alignas(cache_align) uint64_t firstTick_;
  alignas(cache_align) std::atomic<double> durationPerTick_;

 public:
  FastClock() {
    startRealTime_ = system_clock::now();
    startMonoTime_ = steady_clock::now();
    firstTick_ = readTick();
    ::usleep(2000);
    Thread calibrator("FastClock-calibrator", [this]() {
      while (true) {
        sleep_for(seconds(2));
        calibrate();
      }
    });
    calibrator.detach();
  }
  void calibrate() {
    auto duration = steady_clock::now() - startMonoTime_;
    uint64_t tickDiff = readTick() - firstTick_;

    durationPerTick_ =
        std::chrono::duration_cast<AccuracyT>(duration).count() / tickDiff;
  }
  static uint64_t readTick() {
    uint32_t eax, edx;
    __asm__ volatile("rdtsc" : "=a"(eax), "=d"(edx));
    return ((uint64_t)edx) << 32 | (uint64_t)eax;
  }
};
}  // namespace common

#endif  // TINYRPC_FASTCLOCK_HPP
