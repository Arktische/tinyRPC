#include "tcp_server.h"

#include <assert.h>
#include <common/coroutine/coroutine.h>
#include <common/coroutine/coroutine_pool.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>

#include "coroutine_hook.h"
#include "io_thread.h"
#include "tcp_conn_timer.h"
#include "tcp_connection.h"

namespace net {

TcpAcceptor::TcpAcceptor(NetAddress::ptr net_addr) : m_local_addr(net_addr) {
  m_family = m_local_addr->getFamily();
}

void TcpAcceptor::init() {
  m_fd = socket(m_local_addr->getFamily(), SOCK_STREAM, 0);

  assert(m_fd != -1);
  LOG(DEBUG) << "create listenfd succ, listenfd=" << m_fd;

  // int flag = fcntl(fd_, F_GETFL, 0);
  // int rt = fcntl(fd_, F_SETFL, flag | O_NONBLOCK);

  // if (rt != 0) {
  // LOG(ERROR) << "fcntl set nonblock error, errno=" << errno << ", error=" <<
  // strerror(errno);
  // }

  int val = 1;
  if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
    LOG(ERROR) << "set REUSEADDR error";
  }

  socklen_t len = m_local_addr->getSockLen();
  int rt = bind(m_fd, m_local_addr->getSockAddr(), len);
  if (rt != 0) {
    LOG(ERROR) << "bind error, errno=" << errno
               << ", error=" << strerror(errno);
  }
  assert(rt == 0);

  LOG(DEBUG) << "set REUSEADDR succ";
  rt = listen(m_fd, 10);
  if (rt != 0) {
    LOG(ERROR) << "listen error, fd= " << m_fd << ", errno=" << errno
               << ", error=" << strerror(errno);
  }
  assert(rt == 0);
}

TcpAcceptor::~TcpAcceptor() {
  FdEvent::ptr fd_event = FdEventContainer::GetFdContainer()->getFdEvent(m_fd);
  fd_event->unregisterFromReactor();
  if (m_fd != -1) {
    close(m_fd);
  }
}

int TcpAcceptor::toAccept() {
  socklen_t len = 0;
  sockaddr cli_addr;

  // call hook accept
  int rt = accept_hook(m_fd, reinterpret_cast<sockaddr*>(&cli_addr), &len);
  if (rt == -1) {
    LOG(DEBUG) << "error, no new client coming, errno=" << errno
               << "error=" << strerror(errno);
    return -1;
  }

  if (m_family == AF_INET) {
    sockaddr_in* ipv4_addr = reinterpret_cast<sockaddr_in*>(&cli_addr);

    m_peer_addr = std::make_shared<IPAddress>(*ipv4_addr);
  } else if (m_family == AF_UNIX) {
    sockaddr_un* unix_addr = reinterpret_cast<sockaddr_un*>(&cli_addr);

    m_peer_addr = std::make_shared<UnixDomainAddress>(*unix_addr);
  } else {
    LOG(ERROR) << "unknown type protocol!";
    close(rt);
    return -1;
  }

  LOG(INFO) << "New client accepted succ! fd:[" << rt << ", addr:["
            << m_peer_addr->toString() << "]";
  return rt;
}

TcpServer::TcpServer(NetAddress::ptr addr, int pool_size /*=10*/)
    : addr_(addr) {
  io_pool_ = std::make_shared<IOThreadPool>(pool_size);
  //	m_dispatcher = std::make_shared<TinyPbRpcDispacther>();
  main_reactor_ = net::Reactor::GetReactor();
}

void TcpServer::start() {
  acceptor_.reset(new TcpAcceptor(addr_));
  // accpet_coroutine = std::make_shared<net::Coroutine>(128 * 1024,
  // std::bind(&TcpServer::MainAcceptCorFunc, this));

  accpet_coroutine = common::GetCoroutinePool()->getCoroutineInstanse();
  accpet_coroutine->setCallBack(std::bind(&TcpServer::MainAcceptCorFunc, this));

  common::Coroutine::Resume(accpet_coroutine.get());

  // timer_.reset(main_reactor_->getTimer());

  // timer_event_ = std::make_shared<TimerEvent>(10000, true,
  //   std::bind(&TcpServer::MainLoopTimerFunc, this));

  // timer_->addTimerEvent(timer_event_);

  // time_wheel_ = std::make_shared<TcpTimeWheel>(main_reactor_, 6, 10);

  main_reactor_->loop();
}

TcpServer::~TcpServer() {
  common::GetCoroutinePool()->returnCoroutine(accpet_coroutine->getCorId());
  LOG(DEBUG) << "~TcpServer";
}

NetAddress::ptr TcpServer::getPeerAddr() { return acceptor_->getPeerAddr(); }

void TcpServer::MainAcceptCorFunc() {
  LOG(DEBUG) << "enable Hook here";
  // net::enableHook();

  acceptor_->init();
  while (!stop_accept) {
    int fd = acceptor_->toAccept();
    if (fd == -1) {
      LOG(ERROR) << "accept ret -1 error, return, to yield";
      common::Coroutine::Yield();
      continue;
    }
    IOThread* io_thread = io_pool_->getIOThread();
    auto cb = [this, io_thread, fd]() { io_thread->addClient(this, fd); };
    io_thread->getReactor()->addTask(cb);
    tcp_counts_++;
    LOG(DEBUG) << "current tcp connection count is [" << tcp_counts_ << "]";
  }
}

// TinyPbRpcDispacther* TcpServer::getDispatcher() {
//	return m_dispatcher.get();
// }

void TcpServer::AddCoroutine(common::Coroutine::ptr cor) {
  main_reactor_->addCoroutine(cor);
}

}  // namespace net
