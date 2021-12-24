//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_CLOCK_HPP
#define TINYRPC_CLOCK_HPP
#include <sys/types.h>
#include <sys/time.h>
#include <common/fast-clock.hpp>
namespace common {
    class Clock {
        Clock() = delete;
        static int64_t Now() {

        }
    };
}
#endif //TINYRPC_CLOCK_HPP
