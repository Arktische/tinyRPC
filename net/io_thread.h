#ifndef TINYRPC_NET_TCP_IO_THREAD_H
#define TINYRPC_NET_TCP_IO_THREAD_H

#include <common/coroutine/coroutine.h>

#include <map>
#include <memory>

#include "reactor.h"
#include "tcp_conn_timer.h"

namespace net {

class TcpServer;
class TcpConection;

class IOThread {
 public:
  typedef std::shared_ptr<IOThread> ptr;
  IOThread();

  ~IOThread();
  Reactor* getReactor();
  TcpTimeWheel::ptr getTimeWheel();
  bool addClient(TcpServer* tcp_svr, int fd);

 private:
  static void* main(void* arg);

 private:
  void MainLoopTimerFunc();

 private:
  Reactor* reactor_;
  std::map<int, std::shared_ptr<TcpConnection>> clients_;

  TcpTimeWheel::ptr time_wheel_;

  pthread_t thread_;
  pid_t tid_;
  TimerEvent::ptr timer_event_;
};

class IOThreadPool {
 public:
  typedef std::shared_ptr<IOThreadPool> ptr;

  IOThreadPool(int size);

  IOThread* getIOThread();

 private:
  int size_{0};
  int index_{-1};

  std::vector<IOThread::ptr> io_threads_;
};

}  // namespace net

#endif
