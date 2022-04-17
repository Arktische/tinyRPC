//
// Created by tyx on 4/5/22.
//

#ifndef TINYRPC_PROTOCOL_HPP
#define TINYRPC_PROTOCOL_HPP

#include <functional>
#include <system_error>

class Context {};
class Archive {};
template <typename codecT, typename transportT>
class Protocol {
 public:
 Protocol() {
   
 }
 private:
  static codecT codec_;
  static transportT transport_;
};
#endif  // TINYRPC_PROTOCOL_HPP
