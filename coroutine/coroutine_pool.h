#ifndef TINYRPC_COROUTINE_COUROUTINE_POOL_H
#define TINYRPC_COROUTINE_COUROUTINE_POOL_H

#include <vector>

#include "coroutine.h"

namespace common {

class CoroutinePool {
 public:
  CoroutinePool(int pool_size, int stack_size = 1024 * 128);
  ~CoroutinePool();

  Coroutine::ptr getCoroutineInstanse();

  void returnCoroutine(int cor_id);

 private:
  int m_index{0};
  int m_pool_size{0};
  int m_stack_size{0};

  // first--ptr of cor
  // second
  //    false -- can be dispatched
  //    true -- can't be dispatched
  std::vector<std::pair<Coroutine::ptr, bool>> m_free_cors;
};

CoroutinePool* GetCoroutinePool();

}  // namespace common

#endif