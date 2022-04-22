#ifndef TINYRPC_NET_TCP_TCP_CLIENT_H
#define TINYRPC_NET_TCP_TCP_CLIENT_H

#include <common/coroutine/coroutine.h>
#include <google/protobuf/service.h>

#include <memory>

#include "coroutine_hook.h"
#include "net_address.h"
#include "reactor.h"
#include "tcp_connection.h"

namespace net {

class TcpClient {
 public:
  typedef std::shared_ptr<TcpClient> ptr;

  explicit TcpClient(NetAddress::ptr addr);

  ~TcpClient();

  void init();

  std::error_code sendAndRecv();

  void stop();

  TcpConnection* getConnection();

  void setTimeout(const int v) {
    max_timeout_ = v;
    setMaxTimeOut(v);
  }

  void setTryCounts(const int v) { max_retry_ = v; }

 private:
  int family_;
  int fd_{-1};
  int max_retry_{3};    // max try reconnect times
  int max_timeout_{5};  // max connect timeout, s
  bool stop_{false};

  NetAddress::ptr local_addr_{nullptr};
  NetAddress::ptr peer_addr_{nullptr};
  Reactor* reactor_{nullptr};
  TcpConnection::ptr connection_{nullptr};

  bool connected_{false};
};

}  // namespace net

#endif