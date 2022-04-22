#ifndef TINYRPC_NET_TCP_CONN_TIMER_H
#define TINYRPC_NET_TCP_CONN_TIMER_H

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
      : weak_ptr_(ptr), cb_(cb) {}
  ~AbstractSlot() {
    sharedPtr ptr = weak_ptr_.lock();
    if (ptr) {
      cb_(ptr);
    }
  }

 private:
  weakPtr weak_ptr_;
  std::function<void(sharedPtr)> cb_;
};
class TcpTimeWheel {
 public:
  typedef std::shared_ptr<TcpTimeWheel> ptr;

  typedef AbstractSlot<TcpConnection> TcpConnectionSlot;

  TcpTimeWheel(Reactor* reactor, int bucket_count, int invetal = 10);

  ~TcpTimeWheel();

  void fresh(const TcpConnectionSlot::ptr& slot);

  void loopFunc();

 private:
  Reactor* reactor_{nullptr};
  int bucket_count_{0};
  int interval_{0};  // second

  TimerEvent::ptr event_;
  std::queue<std::vector<TcpConnectionSlot::ptr>> wheel_;
};

}  // namespace net

#endif