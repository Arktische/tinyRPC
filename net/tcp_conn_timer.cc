#include "tcp_conn_timer.h"

#include <queue>
#include <vector>

#include "tcp_connection.h"
#include "timer.h"

namespace net {

TcpTimeWheel::TcpTimeWheel(Reactor* reactor, int bucket_count,
                           int inteval /*= 10*/)
    : reactor_(reactor), bucket_count_(bucket_count), interval_(inteval) {
  for (int i = 0; i < bucket_count; ++i) {
    std::vector<TcpConnectionSlot::ptr> tmp;
    wheel_.push(tmp);
  }

  event_ = std::make_shared<TimerEvent>(interval_ * 1000, true,
                                        [this] { loopFunc(); });
  reactor_->getTimer()->addTimerEvent(event_);
}

TcpTimeWheel::~TcpTimeWheel() { reactor_->getTimer()->delTimerEvent(event_); }

void TcpTimeWheel::loopFunc() {
  // DebugLog << "pop src bucket";
  wheel_.pop();
  std::vector<TcpConnectionSlot::ptr> tmp;
  wheel_.push(tmp);
  // DebugLog << "push new bucket";
}

void TcpTimeWheel::fresh(const TcpConnectionSlot::ptr& slot) {
  LOG(DEBUG) << "fresh connection";
  wheel_.back().emplace_back(slot);
}

}  // namespace net