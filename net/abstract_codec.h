#ifndef TINYRPC_NET_CODEC_H
#define TINYRPC_NET_CODEC_H

#include <string>
#include "tcp_buffer.h"
#include "abstract_data.h"


namespace net {

class AbstractCodeC {

 public:

  AbstractCodeC() {}

  virtual ~AbstractCodeC() {}

  virtual void encode(TcpBuffer* buf, AbstractData* data) = 0;

  virtual void decode(TcpBuffer* buf, AbstractData* data) = 0;

  virtual CodeCType type() const = 0;

  static std::string GetCodeCType(CodeCType type) {
    switch (type)
    {
    case CodeCType::CODEC_HTTP:
      return "http";

    case CodeCType::CODEC_TINYPB:
      return "tinypb";

    default:
      return "unknown code type";
    }
}

};

}

#endif
