//
// Created by tyx on 2/13/22.
//

#include "buffer-pool.hpp"

namespace core {

inline int Log2Floor(size_t size) {
#ifdef __GNUC__
  return sizeof(size_t) - 1 - __builtin_clzll(size);
#else
  // TODO: Complete custom implementation
#endif
}

int AlignmentForSize(size_t size) {
  int alignment = kAlignment;
  if (size > kMaxSmallAllocSize) {
    // Cap alignment at kPageSize for large sizes.
    alignment = kPageSize;
  } else if (size >= 128) {
    // Space wasted due to alignment is at most 1/8, i.e., 12.5%.
    alignment = (1 << Log2Floor(size)) / 8;
  } else if (size >= 16) {
    // We need an alignment of at least 16 bytes to satisfy
    // requirements for some SSE types.
    alignment = 16;
  }
  // Maximum alignment allowed is page size alignment.
  if (alignment > kPageSize) {
    alignment = kPageSize;
  }
  return alignment;
}

int ClassIndex(size_t s) {
  if (s <= 1024)
    return (s + 7) >> 3;
  else if (s <= kMaxSmallAllocSize)
    return (s + 127 + (120 << 7)) >> 7;
  return 0;
}

char* ThreadCache::get(size_t size) { return nullptr; }

void ThreadCache::put(char* buf) {}
}  // namespace core