#pragma once
#include <map>
#include <string>

typedef void* (*Constructor)();

class ObjectFactory {
 public:
  static void set(std::string name, Constructor constructor) {
    constructors()[name] = constructor;
  }

  static void* create(const std::string& name) {
    Constructor constructor = NULL;

    if (constructors().find(name) != constructors().end())
      constructor = constructors().find(name)->second;

    if (constructor == NULL) return NULL;

    return (*constructor)();
  }

 private:
  inline static std::map<std::string, Constructor>& constructors() {
    static std::map<std::string, Constructor> instance;
    return instance;
  }
};

#define REGISTER_CLASS(class_name, class_type)                           \
  class class_type##Helper {                                             \
   public:                                                               \
    class_type##Helper() {                                               \
      ObjectFactory::set(#class_name, class_type##Helper::creatObjFunc); \
    }                                                                    \
    static void* creatObjFunc() { return new (class_type); }             \
  };                                                                     \
  class_type##Helper class_type##helper