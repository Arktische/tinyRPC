//
// Created by tyx on 4/5/22.
//

#ifndef TINYRPC_PROTOCOL_HPP
#define TINYRPC_PROTOCOL_HPP

#include <functional>
#include <system_error>

class Context {};
class Archive {};
template <typename codecT>
class Protocol {
 public:
  template <typename requestT, typename responseT>
  using handlerFxT =
      std::function<std::error_code(Context&, requestT&, responseT&)>&;

  template <typename requestT, typename responseT>
  static void Bind(handlerFxT<requestT, responseT> handler) {
    return *this;
  }

  template <typename streamT, typename requestT, typename responseT>
  handlerFxT<requestT, responseT> Process(streamT stream) {}

 private:
  static codecT codec_;
};

void skeleton() { Protocol<Archive> proto_manager; }

#endif  // TINYRPC_PROTOCOL_HPP
