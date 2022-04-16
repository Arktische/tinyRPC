#ifndef TINYRPC_NET_BYTE_H
#define TINYRPC_NET_BYTE_H

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

namespace net {

int32_t getInt32FromNetByte(const char* buf) {
  int32_t tmp;
  memcpy(&tmp, buf, sizeof(tmp));
  return ntohl(tmp);
}

}  // namespace net

#endif
