#include "io_thread.h"

#include <map>
#include <memory>

#include "common/coroutine/coroutine.h"
#include "reactor.h"
#include "tcp_conn_timer.h"
#include "tcp_connection.h"
#include "tcp_server.h"

namespace net {

static thread_local Reactor* t_reactor_ptr = nullptr;

IOThread::IOThread() {
  pthread_create(&thread_, nullptr, &IOThread::main, this);
}

IOThread::~IOThread() {
  reactor_->stop();
  pthread_join(thread_, nullptr);

  if (reactor_ != nullptr) {
    delete reactor_;
    reactor_ = nullptr;
  }
}

Reactor* IOThread::getReactor() { return reactor_; }

TcpTimeWheel::ptr IOThread::getTimeWheel() { return time_wheel_; }

void* IOThread::main(void* arg) {
  assert(t_reactor_ptr == nullptr);
  t_reactor_ptr = new Reactor();
  IOThread* thread = static_cast<IOThread*>(arg);
  thread->reactor_ = t_reactor_ptr;

  thread->timer_event_ = std::make_shared<TimerEvent>(
      10000, true, std::bind(&IOThread::MainLoopTimerFunc, thread));

  thread->getReactor()->getTimer()->addTimerEvent(thread->timer_event_);
  thread->time_wheel_ = std::make_shared<TcpTimeWheel>(thread->reactor_, 2, 10);

  common::Coroutine::GetCurrentCoroutine();

  t_reactor_ptr->loop();

  return nullptr;
}

bool IOThread::addClient(TcpServer* tcp_svr, int fd) {
  auto it = clients_.find(fd);
  if (it != clients_.end()) {
    TcpConnection::ptr s_conn = it->second;
    if (s_conn && s_conn.use_count() > 0 && s_conn->getState() != Closed) {
      LOG(ERROR)
          << "insert error, this fd of TcpConection exist and state not Closed";
      return false;
    }
    // src Tcpconnection can delete
    s_conn.reset();
    it->second.reset();
    // set new Tcpconnection
    it->second = std::make_shared<TcpConnection>(tcp_svr, this, fd, 128,
                                                 tcp_svr->getPeerAddr());
    it->second->registerToTimeWheel();
//    it->second->OnRead(std::move(tcp_svr->rcb_));
//    it->second->OnWrite(std::move(tcp_svr->wcb_));

  } else {
    TcpConnection::ptr conn = std::make_shared<TcpConnection>(
        tcp_svr, this, fd, 128, tcp_svr->getPeerAddr());
    clients_.insert(std::make_pair(fd, conn));
    conn->registerToTimeWheel();
  }
  return true;
}

void IOThread::MainLoopTimerFunc() {
  LOG(DEBUG) << "this IOThread loop timer excute";

  // delete Closed TcpConnection per loop
  // for free memory
  LOG(DEBUG) << "clients_.size=" << clients_.size();
  for (auto& i : clients_) {
    // TcpConnection::ptr s_conn = i.second;
    // DebugLog << "state = " << s_conn->getState();
    if (i.second && i.second.use_count() > 0 &&
        i.second->getState() == Closed) {
      // need to delete TcpConnection
      LOG(DEBUG) << "TcpConection [fd:" << i.first << "] will delete";
      (i.second).reset();
      // s_conn.reset();
    }
  }
}

IOThreadPool::IOThreadPool(int size) : size_(size) {
  io_threads_.resize(size);
  for (int i = 0; i < size; ++i) {
    io_threads_[i] = std::make_shared<IOThread>();
  }
}

IOThread* IOThreadPool::getIOThread() {
  if (index_ == size_ || index_ == -1) {
    index_ = 0;
  }
  return io_threads_[index_++].get();
}

}  // namespace net