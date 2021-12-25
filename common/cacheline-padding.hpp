//
// Created by tyx on 12/24/21.
//

#ifndef TINYRPC_CACHELINE_PADDING_HPP
#define TINYRPC_CACHELINE_PADDING_HPP
#define CACHE_LINE_SIZE 128
#define CONCAT_(a, b) a##b
#define CONCAT(a,b) CONCAT_(a,b)
#define CACHE_LINE_PADDING(var_name)                                           \
  var_name;                                                                    \
  char CONCAT(__padding__,                                                     \
              __LINE__)[CACHE_LINE_SIZE > sizeof(decltype(var_name))           \
                            ? CACHE_LINE_SIZE - sizeof(decltype(var_name))     \
                            : 1]
#endif // TINYRPC_CACHELINE_PADDING_HPP
