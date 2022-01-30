//
// Created by tyx on 2022/1/30.
//
#pragma once
#ifndef TINYRPC_FASTMEMCPY_HPP
#define TINYRPC_FASTMEMCPY_HPP
#include <immintrin.h>

#include <cstring>
namespace common {
inline void memcpy(char* dst, const char* src, size_t size) {
  if (!(size >> 8)) {
    std::memcpy(dst, src, size);
    return;
  }
  auto p = reinterpret_cast<const char*>((reinterpret_cast<size_t>(src) + 31) &
                                         (~31ll));
  auto diff = p - src;
  std::memcpy(dst, src, diff);
  dst += diff;
  src += diff;
  size -= diff;
  size_t tms = size >> 5;
  while (tms--) {
    __m256i _src = _mm256_load_si256(reinterpret_cast<const __m256i*>(src));
    _mm256_store_si256(reinterpret_cast<__m256i*>(dst), _src);
    dst += 16;
    src += 16;
  }
  std::memcpy(dst, src, size & 0x1f);
}
}  // namespace common
#endif  // TINYRPC_FASTMEMCPY_HPP
