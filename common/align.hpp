//
// Created by tyx on 12/24/21.
//

#ifndef TINYRPC_CACHELINE_ALIGN_HPP
#define TINYRPC_CACHELINE_ALIGN_HPP
#include <cstddef>
namespace common {
const std::size_t hw_destructive_interference_size =
#ifdef __x86_64__
    128;
#elifdef __arm__
    64;
#endif

} // namespace common
#endif // TINYRPC_CACHELINE_ALIGN_HPP
