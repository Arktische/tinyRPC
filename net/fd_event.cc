#include "fd_event.h"

#include <fcntl.h>
#include <unistd.h>

namespace net {

static FdEventContainer* g_FdContainer = nullptr;

FdEvent::FdEvent(Reactor* reactor, int fd /*=-1*/)
    : m_fd(fd), m_reactor(reactor) {
  if (reactor == nullptr) {
    LOG(ERROR) << "create reactor first";
  }
  assert(reactor != nullptr);
}

FdEvent::FdEvent(int fd) : m_fd(fd) {}

FdEvent::~FdEvent() {}

void FdEvent::handleEvent(int flag) {
  if (flag == READ) {
    read_cb_();
  } else if (flag == WRITE) {
    write_cb_();
  } else {
    LOG(ERROR) << "error flag";
  }
}

void FdEvent::setCallBack(IOEvent flag, std::function<void()> cb) {
  if (flag == READ) {
    read_cb_ = cb;
  } else if (flag == WRITE) {
    write_cb_ = cb;
  } else {
    LOG(ERROR) << "error flag";
  }
}

std::function<void()> FdEvent::getCallBack(IOEvent flag) const {
  if (flag == READ) {
    return read_cb_;
  } else if (flag == WRITE) {
    return write_cb_;
  }
  return nullptr;
}

void FdEvent::addListenEvents(IOEvent event) {
  if (listen_events_ & event) {
    LOG(DEBUG) << "already has this event, skip";
    return;
  }
  listen_events_ |= event;
  updateToReactor();
  // DebugLog << "add succ";
}

void FdEvent::delListenEvents(IOEvent event) {
  if (listen_events_ & event) {
    LOG(DEBUG) << "delete succ";
    listen_events_ &= ~event;
    updateToReactor();
    return;
  }
  LOG(ERROR) << "this event not exist, skip";
}

void FdEvent::updateToReactor() {
  epoll_event event;
  event.events = listen_events_;
  event.data.ptr = this;
  // DebugLog << "reactor = " << reactor_ << "log tid_ =" <<
  // reactor_->getTid();

  m_reactor->addEvent(m_fd, event);
}

void FdEvent::unregisterFromReactor() {
  m_reactor->delEvent(m_fd);
  listen_events_ = 0;
  read_cb_ = nullptr;
  write_cb_ = nullptr;
}

int FdEvent::getFd() const { return m_fd; }

void FdEvent::setFd(const int fd) { m_fd = fd; }

int FdEvent::getListenEvents() const { return listen_events_; }

Reactor* FdEvent::getReactor() const { return m_reactor; }

void FdEvent::setReactor(Reactor* r) { m_reactor = r; }

void FdEvent::setNonBlock() {
  if (m_fd == -1) {
    LOG(ERROR) << "error, fd=-1";
    return;
  }

  int flag = fcntl(m_fd, F_GETFL, 0);
  if (flag & O_NONBLOCK) {
    LOG(DEBUG) << "fd already set o_nonblock";
    return;
  }

  fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
  flag = fcntl(m_fd, F_GETFL, 0);
  if (flag & O_NONBLOCK) {
    LOG(DEBUG) << "succ set o_nonblock";
  } else {
    LOG(ERROR) << "set o_nonblock error";
  }
}

bool FdEvent::isNonBlock() {
  if (m_fd == -1) {
    LOG(ERROR) << "error, fd=-1";
    return false;
  }
  int flag = fcntl(m_fd, F_GETFL, 0);
  return (flag & O_NONBLOCK);
}

FdEvent::ptr FdEventContainer::getFdEvent(int fd) {
  RWMutex::ReadLock lock(mutex_);
  std::vector<FdEvent::ptr> tmps = fds_;
  lock.unlock();
  if (fd < static_cast<int>(tmps.size())) {
    return tmps[fd];
  }

  int n = (int)(tmps.size() * 1.5);
  n = (n > fd ? n : fd);

  for (int i = tmps.size(); i < n; ++i) {
    FdEvent::ptr p = std::make_shared<FdEvent>(i);
    tmps.push_back(p);
  }
  FdEvent::ptr re = tmps[fd];
  RWMutex::WriteLock lock2(mutex_);
  fds_.swap(tmps);
  lock.unlock();
  return re;
}

FdEventContainer::FdEventContainer(int size) {
  for (int i = 0; i < size; ++i) {
    FdEvent::ptr p = std::make_shared<FdEvent>(i);
    fds_.push_back(p);
  }
}

FdEventContainer* FdEventContainer::GetFdContainer() {
  if (g_FdContainer == nullptr) {
    g_FdContainer = new FdEventContainer(128);
  }
  return g_FdContainer;
}

}  // namespace net
