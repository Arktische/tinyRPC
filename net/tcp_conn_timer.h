#ifndef TINYRPC_NET_TCP_TCPCONNECTIONTIMEWHEEL_H
#define TINYRPC_NET_TCP_TCPCONNECTIONTIMEWHEEL_H

#include <queue>
#include <vector>

#include "reactor.h"
#include "timer.h"

namespace net {

class TcpConnection;
template <class T>
class AbstractSlot {
 public:
  typedef std::shared_ptr<AbstractSlot> ptr;
  typedef std::weak_ptr<T> weakPtr;
  typedef std::shared_ptr<T> sharedPtr;

  AbstractSlot(weakPtr ptr, std::function<void(sharedPtr)> cb)
      : m_weak_ptr(ptr), m_cb(cb) {}
  ~AbstractSlot() {
    sharedPtr ptr = m_weak_ptr.lock();
    if (ptr) {
      m_cb(ptr);
    }
  }

 private:
  weakPtr m_weak_ptr;
  std::function<void(sharedPtr)> m_cb;
};
class TcpTimeWheel {
 public:
  typedef std::shared_ptr<TcpTimeWheel> ptr;

  typedef AbstractSlot<TcpConnection> TcpConnectionSlot;

  TcpTimeWheel(Reactor* reactor, int bucket_count, int invetal = 10);

  ~TcpTimeWheel();

  void fresh(TcpConnectionSlot::ptr slot);

  void loopFunc();

 private:
  Reactor* m_reactor{nullptr};
  int m_bucket_count{0};
  int m_inteval{0};  // second

  TimerEvent::ptr m_event;
  std::queue<std::vector<TcpConnectionSlot::ptr>> m_wheel;
};

}  // namespace net

#endif