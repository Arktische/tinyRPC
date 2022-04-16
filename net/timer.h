#ifndef TINYRPC_NET_TIMER_H
#define TINYRPC_NET_TIMER_H

#include <time.h>
#include <memory>
#include <map>
#include <functional>
#include "mutex.h"
#include "reactor.h"
#include "fd_event.h"
#include "common/log.hpp"


namespace net {

int64_t getNowMs();


class TimerEvent {

 public:

  typedef std::shared_ptr<TimerEvent> ptr;
  TimerEvent(int64_t interval, bool is_repeated, std::function<void()>task)
    : m_interval(interval), m_is_repeated(is_repeated), m_task(task) {
    m_arrive_time = getNowMs() + m_interval;  	
    // DebugLog << "timeevent will occur at " << m_arrive_time;
  }

  void resetTime() {
    // DebugLog << "reset tiemrevent, origin arrivetime=" << m_arrive_time;
    m_arrive_time = getNowMs() + m_interval;  	
    // DebugLog << "reset tiemrevent, now arrivetime=" << m_arrive_time;
  }

 public:
  int64_t m_arrive_time;   // when to excute task, ms
  int64_t m_interval;     // interval between two tasks, ms
  bool m_is_repeated {false};
	bool m_is_cancled {false};
  std::function<void()> m_task;

};

class FdEvent;

class Timer : public FdEvent {

 public:

  typedef std::shared_ptr<Timer> ptr;
  
  Timer(Reactor* reactor);

	~Timer();

	void addTimerEvent(TimerEvent::ptr event, bool need_reset = true);

	void delTimerEvent(TimerEvent::ptr event);

	void resetArriveTime();

  void onTimer();

 private:

 	std::multimap<int64_t, TimerEvent::ptr> m_pending_events;


};



}

#endif
