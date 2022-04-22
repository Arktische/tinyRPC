#ifndef net_NET_TCP_TCP_CONNECTION_H
#define net_NET_TCP_TCP_CONNECTION_H

#include <common/coroutine/coroutine.h>

#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include <common/log.hpp>

#include "fd_event.h"
#include "io_thread.h"
#include "net_address.h"
#include "reactor.h"
#include "tcp_buffer.h"
#include "tcp_conn_timer.h"

namespace net {

class TcpServer;
class TcpClient;
class IOThread;

enum TcpConnectionState {
  NotConnected = 1,  // can do io
  Connected = 2,     // can do io
  HalfClosing =
      3,  // server call shutdown, write half close. can read,but can't write
  Closed = 4,  // can't do io
};

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  typedef std::shared_ptr<TcpConnection> ptr;

  TcpConnection(net::TcpServer* tcp_svr, net::IOThread* io_thread, int fd,
                int buff_size, NetAddress::ptr peer_addr);

  TcpConnection(net::TcpClient* tcp_cli, net::Reactor* reactor, int fd,
                int buff_size, NetAddress::ptr peer_addr);

  void setUpClient();

  ~TcpConnection();

  void initBuffer(int size);

  enum ConnectionType {
    ServerConnection = 1,  // owned by tcp_server
    ClientConnection = 2,  // owned by tcp_client
  };

 public:
  void shutdownConnection();

  TcpConnectionState getState() const;

  TcpBuffer* getInBuffer();

  TcpBuffer* getOutBuffer();

  //  TinyPbCodeC* getCodec() const;

  //  bool getResPackageData(TinyPbStruct& pb_struct);

  void registerToTimeWheel();

 public:
  void MainServerLoopCorFunc();

  void input();
  void OnRead(std::function<void(ptr)> f) { rcb_ = std::move(f); }

  void execute();

  void output();
  void OnWrite(std::function<void(ptr)> f) { wcb_ = std::move(f); }

 private:
  void clearClient();

 private:
  TcpServer* tcp_svr_{nullptr};
  TcpClient* tcp_cli_{nullptr};
  IOThread* io_thread_{nullptr};
  Reactor* reactor_{nullptr};

  int fd_{-1};
  TcpConnectionState state_{TcpConnectionState::Connected};
  ConnectionType conn_type_{ServerConnection};

  NetAddress::ptr peer_addr_;

  TcpBuffer::ptr read_buffer_;
  TcpBuffer::ptr write_buffer_;

  common::Coroutine::ptr loop_coroutine_;

  FdEvent::ptr fd_event_;
  bool stop_{false};

  std::weak_ptr<AbstractSlot<TcpConnection>> weak_slot_;
  std::function<void(ptr)> rcb_;
  std::function<void(ptr)> wcb_;
};

}  // namespace net

#endif
