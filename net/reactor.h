#ifndef TINYRPC_NET_EVENT_LOOP_H
#define TINYRPC_NET_EVENT_LOOP_H

#include <common/coroutine/coroutine.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <functional>
#include <map>
#include <vector>

#include "mutex.h"

namespace net {

class FdEvent;
class Timer;

// typedef std::shared_ptr<Timer> TimerPtr;

class Reactor {
 public:
  typedef std::shared_ptr<Reactor> ptr;

    Reactor();

  ~Reactor();

  void addEvent(int fd, epoll_event event, bool is_wakeup = true);

  void delEvent(int fd, bool is_wakeup = true);

  void addTask(std::function<void()> task, bool is_wakeup = true);

  void addTask(std::vector<std::function<void()>> task, bool is_wakeup = true);

  void addCoroutine(common::Coroutine::ptr cor, bool is_wakeup = true);

  void wakeup();

  void loop();

  void stop();

  Timer* getTimer();

  pid_t getTid();

 public:
  static Reactor* GetReactor();

 private:
  void addWakeupFd();

  bool isLoopThread() const;

  void addEventInLoopThread(int fd, epoll_event event);

  void delEventInLoopThread(int fd);

 private:
  int epfd_{-1};
  int wake_fd_{-1};   // wakeup fd
  int timer_fd_{-1};  // timer fd
  bool stop_flag_{false};
  bool looping_{false};
  bool init_timer_{false};
  pid_t tid_{0};  // thread id

  Mutex mutex_;  // mutex

  std::vector<int> fds_;  // alrady care events
  std::atomic<int> fd_size_;

  // fds that wait for operate
  // 1 -- to add to loop
  // 2 -- to del from loop
  std::map<int, epoll_event> pending_add_fds_;
  std::vector<int> pending_del_fds_;
  std::vector<std::function<void()>> pending_tasks_;

  Timer* timer_;
};

}  // namespace net

#endif
