#include "tcp_client.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include "common/coroutine/coroutine.h"
#include "common/coroutine/coroutine_pool.h"
#include "common/error.hpp"
#include "common/log.hpp"
#include "coroutine_hook.h"
#include "net_address.h"

namespace net {

TcpClient::TcpClient(NetAddress::ptr addr) : peer_addr_(addr) {
  family_ = peer_addr_->getFamily();
  fd_ = socket(AF_INET, SOCK_STREAM, 0);

  reactor_ = Reactor::GetReactor();
  connection_ =
      std::make_shared<TcpConnection>(this, reactor_, fd_, 128, peer_addr_);
  assert(reactor_ != nullptr);
}

TcpClient::~TcpClient() {
  if (fd_ > 0) {
    close(fd_);
  }
}

TcpConnection* TcpClient::getConnection() {
  if (!connection_.get()) {
    connection_ =
        std::make_shared<TcpConnection>(this, reactor_, fd_, 128, peer_addr_);
  }
  return connection_.get();
}

std::error_code TcpClient::sendAndRecv() {
  if (connection_->getState() != Connected) {
    int n = max_retry_;
    while (n > 0) {
      int rt = connect_hook(
          fd_, reinterpret_cast<sockaddr*>(peer_addr_->getSockAddr()),
          peer_addr_->getSockLen());
      if (rt == 0) {
        LOG(DEBUG) << "connect [" << peer_addr_->toString() << "] succ!";
        connection_->setUpClient();
        break;
      }
      n--;
    }
  }
  if (connection_->getState() != Connected) {
    return NetError::NET_CONN_FAILED;
  }
  connection_->setUpClient();
  connection_->output();

  connection_->input();
  connection_->execute();

  // if (!connection_->getResPackageData()) {
  //   return ERROR_FAILED_GET_REPLY;
  // }
  return NetError::NET_NOERROR;
}

void TcpClient::stop() {
  if (!stop_) {
    stop_ = true;
    reactor_->stop();
  }
}

}  // namespace net
