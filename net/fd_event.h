#ifndef TINYRPC_NET_FD_EVNET_H
#define TINYRPC_NET_FD_EVNET_H

#include <common/coroutine/coroutine.h>
#include <net/mutex.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cassert>
#include <functional>
#include <memory>

#include <common/log.hpp>

#include "reactor.h"

namespace net {

class Reactor;

enum IOEvent {
  READ = EPOLLIN,
  WRITE = EPOLLOUT,
  ETModel = EPOLLET,
};

class FdEvent : public std::enable_shared_from_this<FdEvent> {
 public:
  typedef std::shared_ptr<FdEvent> ptr;

  FdEvent(Reactor* reactor, int fd = -1);

  FdEvent(int fd);

  virtual ~FdEvent();

  void handleEvent(int flag);

  void setCallBack(IOEvent flag, std::function<void()> cb);

  std::function<void()> getCallBack(IOEvent flag) const;

  void addListenEvents(IOEvent event);

  void delListenEvents(IOEvent event);

  void updateToReactor();

  void unregisterFromReactor();

  int getFd() const;

  void setFd(const int fd);

  int getListenEvents() const;

  Reactor* getReactor() const;

  void setReactor(Reactor* r);

  void setNonBlock();

  bool isNonBlock();

 public:
  Mutex mutex_;

 protected:
  int m_fd{-1};
  std::function<void()> read_cb_;
  std::function<void()> write_cb_;

  int listen_events_{0};
  int cur_events_{0};

  Reactor* m_reactor{nullptr};
};

class FdEventContainer {
 public:
  FdEventContainer(int size);

  FdEvent::ptr getFdEvent(int fd);

 public:
  static FdEventContainer* GetFdContainer();

 private:
  RWMutex mutex_;
  std::vector<FdEvent::ptr> fds_;
};

}  // namespace net

#endif
