#pragma once
#include <map>
#include <string>

typedef void* (*Constructor)();

class ObjectFactory {
 public:
  static auto set(const std::string& name, Constructor constructor) {
    constructorMap()[name] = constructor;
  }

  static auto create(const std::string& name) -> void* {
    Constructor constructor = nullptr;

    if (constructorMap().find(name) != constructorMap().end())
      constructor = constructorMap().find(name)->second;

    if (constructor == nullptr) return nullptr;

    return (*constructor)();
  }

 private:
  inline static auto constructorMap() -> std::map<std::string, Constructor>& {
    static std::map<std::string, Constructor> instance;
    return instance;
  }
};

#define register_class(class_name, class_type)                           \
  class class_type##Helper {                                             \
   public:                                                               \
    class_type##Helper() {                                               \
      ObjectFactory::set(#class_name, class_type##Helper::creatObjFunc); \
    }                                                                    \
    static auto creatObjFunc() -> void* { return new (class_type); }     \
  };                                                                     \
  class_type##Helper class_type##helper