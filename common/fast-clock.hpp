//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_FAST_CLOCK_HPP
#define TINYRPC_FAST_CLOCK_HPP
// #include <sys/time.h>
#include <time.h>
#include <chrono>
class FastClock {
public:
    FastClock() {
        auto ret = clock_gettime(CLOCK_REALTIME,&startRealTime_);
        startMonoTime_ = std::chrono::steady_clock::now();
        firstTick_ = read();
    }

    static uint64_t read() {
        uint32_t eax,edx;
        __asm__ volatile ("rdtsc" : "=a" (eax), "=d" (edx));
        return ((uint64_t)edx) << 32 | (uint64_t)eax;
    }
private:
    timespec startRealTime_;
    std::chrono::steady_clock::time_point startMonoTime_;
    uint64_t firstTick_;
};
#endif //TINYRPC_FAST_CLOCK_HPP
