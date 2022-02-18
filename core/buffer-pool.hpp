//
// Created by tyx on 2/13/22.
//
#pragma once
#ifndef TINYRPC_BUFFER_POOL_HPP
#define TINYRPC_BUFFER_POOL_HPP
#include <vector>
#include <set>
#include <queue>
#include <bitset>
#include <list>
#include <climits>
#include <common/non-copyable.hpp>
namespace core {
using std::pair;
using std::list;
using std::vector;
using std::queue;
using std::bitset;
using common::NonCopyable;
struct BufferPool : NonCopyable{

  static const size_t kBlockSize = 4096;

  BufferPool() = default;

  vector<char*> bufferIndex_; // bufID as index
  char* get(size_t size) {
    if(size < kBlockSize) {return localCache_;}
  }
  void putback() {

  }
  static thread_local char localCache_[kBlockSize];
  list<char*> availList_[kBlockSize];
};
static_assert((BufferPool::kBlockSize&(BufferPool::kBlockSize-1)) ==0, "invalid block size, must be power of 2");
thread_local char BufferPool::localCache_[BufferPool::kBlockSize]{0};
}
#endif  // TINYRPC_BUFFER_POOL_HPP
