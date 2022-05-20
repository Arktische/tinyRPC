#pragma once
#include "schema.hpp"
template <typename codec_type>
struct RemoteProcedureBase {
  virtual auto call(codec_type) -> codec_type = 0;
};

#define rpc(request, response, handler, codec)                              \
  struct __remote__##handler : RemoteProcedureBase<codec> {                 \
    auto call(codec codec_) -> codec {                                      \
      request req;                                                          \
      for_each(req, [&codec_](auto&&, auto&& value) { codec_ >> value; });  \
      response resp = handler(req);                                         \
      for_each(resp, [&codec_](auto&&, auto&& value) { codec_ << value; }); \
      return codec_;                                                        \
    }                                                                       \
  };                                                                        \
  register_class(handler, __remote__##handler)

#define dispatch(method_str, codec) \
  (static_cast<RemoteProcedureBase<codec>*>(ObjectFactory::create(method_str)))