//
// Created by tyx on 12/23/21.
//

#ifndef TINYRPC_SINGLETON_HPP
#define TINYRPC_SINGLETON_HPP
template <class T> class Singleton {
protected:
  Singleton() = default;
  virtual ~Singleton() = default;
public:
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;
  Singleton(T&&) = delete;
public:
  static T &getInstance() {
    static T instance;
    return instance;
  }
};
#endif // TINYRPC_SINGLETON_HPP
