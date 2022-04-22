#include "timer.h"

#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <time.h>

#include <functional>
#include <map>
#include <vector>

#include "common/log.hpp"
#include "coroutine_hook.h"
#include "fd_event.h"
#include "mutex.h"

extern read_fun_ptr_t g_sys_read_fun;  // sys read func

namespace net {

int64_t getNowMs() {
  timeval val;
  gettimeofday(&val, nullptr);
  int64_t re = val.tv_sec * 1000 + val.tv_usec / 1000;
  return re;
}

Timer::Timer(Reactor* reactor) : FdEvent(reactor) {
  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  LOG(DEBUG) << "timer_ fd = " << m_fd;
  if (m_fd == -1) {
    LOG(DEBUG) << "timerfd_create error";
  }
  // DebugLog << "timerfd is [" << fd_ << "]";
  read_cb_ = std::bind(&Timer::onTimer, this);
  addListenEvents(READ);
  // updateToReactor();
}

Timer::~Timer() {
  unregisterFromReactor();
  close(m_fd);
}

void Timer::addTimerEvent(TimerEvent::ptr event, bool need_reset /*=true*/) {
  bool is_reset = false;
  if (m_pending_events.empty()) {
    is_reset = true;
  } else {
    auto it = m_pending_events.begin();
    if (event->m_arrive_time < (*it).second->m_arrive_time) {
      is_reset = true;
    }
  }
  m_pending_events.emplace(event->m_arrive_time, event);
  if (is_reset && need_reset) {
    // DebugLog << "need reset timer";
    resetArriveTime();
  }
  // DebugLog << "add timer event succ";
}

void Timer::delTimerEvent(TimerEvent::ptr event) {
  event->m_is_cancled = true;
  // DebugLog << "del timer event succ";
}

void Timer::resetArriveTime() {
  if (m_pending_events.size() == 0) {
    LOG(DEBUG) << "no timerevent pending, size = 0";
    return;
  }

  int64_t now = getNowMs();
  auto it = m_pending_events.begin();
  if ((*it).first < now) {
    LOG(DEBUG) << "all timer events has already expire";
    return;
  }
  int64_t interval = (*it).first - now;

  itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));

  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = interval / 1000;
  ts.tv_nsec = (interval % 1000) * 1000000;
  new_value.it_value = ts;

  int rt = timerfd_settime(m_fd, 0, &new_value, nullptr);

  if (rt != 0) {
    LOG(ERROR) << "tiemr_settime error, interval=" << interval;
  } else {
    // DebugLog << "reset timer succ, next occur time=" << (*it).first;
  }
}

void Timer::onTimer() {
  LOG(DEBUG) << "onTimer, first read data";
  char buf[8];
  while (1) {
    if ((g_sys_read_fun(m_fd, buf, 8) == -1) && errno == EAGAIN) {
      break;
    }
  }

  int64_t now = getNowMs();
  auto it = m_pending_events.begin();
  std::vector<TimerEvent::ptr> tmps;
  std::vector<std::function<void()>> tasks;
  for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
    if ((*it).first <= now && !((*it).second->m_is_cancled)) {
      tmps.push_back((*it).second);
      tasks.push_back((*it).second->m_task);
    } else {
      break;
    }
  }

  assert(m_reactor != nullptr);
  m_reactor->addTask(tasks);
  m_pending_events.erase(m_pending_events.begin(), it);
  for (auto i = tmps.begin(); i != tmps.end(); ++i) {
    if ((*i)->m_is_repeated) {
      (*i)->resetTime();
      addTimerEvent(*i, false);
    }
  }

  resetArriveTime();
}

}  // namespace net
