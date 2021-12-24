//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_SINGLETON_HPP
#define TINYRPC_SINGLETON_HPP
template <class T> class Singleton {
protected:
  Singleton() = default;

public:
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
  ~Singleton() = default;

public:
  static T &getInstance() {
    static T inst;
    return inst;
  }
};
#endif // TINYRPC_SINGLETON_HPP
