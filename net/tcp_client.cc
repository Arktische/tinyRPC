#include <sys/socket.h>
#include <arpa/inet.h>
#include "common/log.hpp"
#include "coroutine/coroutine.h"
#include "coroutine/coroutine_hook.h"
#include "coroutine/coroutine_pool.h"
#include "net_address.h"
#include "tcp_client.h" 
#include "common/error.hpp"

namespace net {

TcpClient::TcpClient(NetAddress::ptr addr) : m_peer_addr(addr) {

  m_family = m_peer_addr->getFamily();
  m_fd = socket(AF_INET, SOCK_STREAM, 0);

  m_reactor = Reactor::GetReactor();
  m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
  assert(m_reactor != nullptr);

}

TcpClient::~TcpClient() {
  if (m_fd > 0) {
    close(m_fd);
  }
}

TcpConnection* TcpClient::getConnection() {
  if (!m_connection.get()) {
    m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
  }
  return m_connection.get();
}

std::error_code TcpClient::sendAndRecv() {
  if (m_connection->getState() != Connected) {
    int n = m_try_counts;
    while (n > 0) {
      int rt = connect_hook(m_fd, reinterpret_cast<sockaddr*>(m_peer_addr->getSockAddr()), m_peer_addr->getSockLen());
      if (rt == 0) {
        LOG(DEBUG) << "connect [" << m_peer_addr->toString() << "] succ!";
        m_connection->setUpClient();
        break;
      }
      n--;
    }
  }
  if (m_connection->getState() != Connected) {
    return NetError::NET_CONN_FAILED;
  }
  m_connection->setUpClient();
  m_connection->output();

  m_connection->input();
  m_connection->execute();

  // if (!m_connection->getResPackageData()) {
  //   return ERROR_FAILED_GET_REPLY;
  // }
  return NetError::NET_NOERROR;
}

void TcpClient::stop() {
  if (!m_is_stop) {
    m_is_stop = true;
    m_reactor->stop();
  }
}

} // namespace name
