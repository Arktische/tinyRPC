#include "reactor.h"

#include <assert.h>
#include <common/coroutine/coroutine.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#include <algorithm>

#include "common/log.hpp"
#include "coroutine_hook.h"
#include "fd_event.h"
#include "mutex.h"
#include "timer.h"

extern read_fun_ptr_t g_sys_read_fun;    // sys read func
extern write_fun_ptr_t g_sys_write_fun;  // sys write func

namespace net {

static thread_local Reactor* t_reactor_ptr = nullptr;

static thread_local int t_max_epoll_timeout = 10000;  // ms

Reactor::Reactor() {
  // one thread can't create more than one reactor object!!
  assert(t_reactor_ptr == nullptr);
  tid_ = gettid();

  LOG(DEBUG) << "thread[" << tid_ << "] succ create a reactor object";
  t_reactor_ptr = this;

  if ((epfd_ = epoll_create(1)) <= 0) {
    LOG(ERROR) << "epoll_create error";
  } else {
    LOG(DEBUG) << "epfd_ = " << epfd_;
  }
  assert(epfd_ > 0);

  if ((wake_fd_ = eventfd(0, EFD_NONBLOCK)) <= 0) {
    LOG(ERROR) << "eventfd error";
  }
  LOG(DEBUG) << "wakefd = " << wake_fd_;
  assert(wake_fd_ > 0);
  addWakeupFd();
}

Reactor::~Reactor() {
  LOG(DEBUG) << "~Reactor";
  close(epfd_);
  if (timer_ != nullptr) {
    delete timer_;
    timer_ = nullptr;
  }
  t_reactor_ptr = nullptr;
}

Reactor* Reactor::GetReactor() {
  if (t_reactor_ptr == nullptr) {
    LOG(DEBUG) << "Create new Reactor";
    t_reactor_ptr = new Reactor();
  }
  // DebugLog << "t_reactor_ptr = " << t_reactor_ptr;
  return t_reactor_ptr;
}

// call by other threads, need lock
void Reactor::addEvent(int fd, epoll_event event, bool is_wakeup /*=true*/) {
  if (fd == -1) {
    LOG(ERROR) << "add error. fd invalid, fd = -1";
    return;
  }
  if (isLoopThread()) {
    addEventInLoopThread(fd, event);
    return;
  }
  {
    Mutex::Lock lock(mutex_);
    pending_add_fds_.insert(std::pair<int, epoll_event>(fd, event));
  }
  if (is_wakeup) {
    wakeup();
  }
}

// call by other threads, need lock
void Reactor::delEvent(int fd, bool is_wakeup /*=true*/) {
  if (fd == -1) {
    LOG(ERROR) << "add error. fd invalid, fd = -1";
    return;
  }

  if (isLoopThread()) {
    delEventInLoopThread(fd);
    return;
  }

  {
    Mutex::Lock lock(mutex_);
    pending_del_fds_.push_back(fd);
  }
  if (is_wakeup) {
    wakeup();
  }
}

void Reactor::wakeup() {
  if (!looping_) {
    return;
  }

  uint64_t tmp = 1;
  uint64_t* p = &tmp;
  if (g_sys_write_fun(wake_fd_, p, 8) != 8) {
    LOG(ERROR) << "write wakeupfd[" << wake_fd_ << "] error";
  }
}

// tid_ only can be writed in Reactor::Reactor, so it needn't to lock
bool Reactor::isLoopThread() const {
  if (tid_ == gettid()) {
    // DebugLog << "return true";
    return true;
  }
  // DebugLog << "tid_ = "<< tid_ << ", getttid = " << gettid() <<"return
  // false";
  return false;
}

void Reactor::addWakeupFd() {
  int op = EPOLL_CTL_ADD;
  epoll_event event;
  event.data.fd = wake_fd_;
  event.events = EPOLLIN;
  if ((epoll_ctl(epfd_, op, wake_fd_, &event)) != 0) {
    LOG(ERROR) << "epoo_ctl error, fd[" << wake_fd_ << "], errno=" << errno
               << ", err=" << strerror(errno);
  }
  fds_.push_back(wake_fd_);
}

// need't mutex, only this thread call
void Reactor::addEventInLoopThread(int fd, epoll_event event) {
  assert(isLoopThread());

  int op = EPOLL_CTL_ADD;
  bool is_add = true;
  // int tmp_fd = event;
  auto it = find(fds_.begin(), fds_.end(), fd);
  if (it != fds_.end()) {
    is_add = false;
    op = EPOLL_CTL_MOD;
  }

  // epoll_event event;
  // event.data.ptr = fd_event.get();
  // event.events = fd_event->getListenEvents();

  if (epoll_ctl(epfd_, op, fd, &event) != 0) {
    LOG(ERROR) << "epoo_ctl error, fd[" << fd << "]";
    return;
  }
  if (is_add) {
    fds_.push_back(fd);
  }
  LOG(DEBUG) << "epoll_ctl add succ, fd[" << fd << "]";
}

// need't mutex, only this thread call
void Reactor::delEventInLoopThread(int fd) {
  assert(isLoopThread());

  auto it = find(fds_.begin(), fds_.end(), fd);
  if (it == fds_.end()) {
    LOG(DEBUG) << "fd[" << fd << "] not in this loop";
  }
  int op = EPOLL_CTL_DEL;

  if ((epoll_ctl(epfd_, op, fd, nullptr)) != 0) {
    LOG(ERROR) << "epoo_ctl error, fd[" << fd << "]";
  }

  fds_.erase(it);
  LOG(DEBUG) << "del succ, fd[" << fd << "]";
}

void Reactor::loop() {
  assert(isLoopThread());
  if (looping_) {
    // DebugLog << "this reactor is looping!";
    return;
  }

  looping_ = true;
  stop_flag_ = false;

  while (!stop_flag_) {
    const int MAX_EVENTS = 10;
    epoll_event re_events[MAX_EVENTS + 1];
    // DebugLog << "task";
    // excute tasks
    for (size_t i = 0; i < pending_tasks_.size(); ++i) {
      // DebugLog << "begin to excute task[" << i << "]";
      pending_tasks_[i]();
      // DebugLog << "end excute tasks[" << i << "]";
    }
    pending_tasks_.clear();
    LOG(DEBUG) << "to epoll_wait";
    int rt = epoll_wait(epfd_, re_events, MAX_EVENTS, t_max_epoll_timeout);

    // DebugLog << "epoll_waiti back";

    if (rt < 0) {
      LOG(ERROR) << "epoll_wait error, skip";
    } else {
      // DebugLog << "epoll_wait back, rt = " << rt;
      for (int i = 0; i < rt; ++i) {
        epoll_event one_event = re_events[i];

        if (one_event.data.fd == wake_fd_ && (one_event.events & READ)) {
          // wakeup
          // DebugLog << "epoll wakeup, fd=[" << wake_fd_ << "]";
          char buf[8];
          while (1) {
            if ((g_sys_read_fun(wake_fd_, buf, 8) == -1) && errno == EAGAIN) {
              break;
            }
          }

        } else {
          auto* ptr = (FdEvent*)one_event.data.ptr;
          if (ptr != nullptr) {
            int fd;
            std::function<void()> read_cb;
            std::function<void()> write_cb;

            {
              Mutex::Lock lock(ptr->mutex_);
              fd = ptr->getFd();
              read_cb = ptr->getCallBack(READ);
              write_cb = ptr->getCallBack(WRITE);
            }

            if ((!(one_event.events & EPOLLIN)) &&
                (!(one_event.events & EPOLLOUT))) {
              LOG(DEBUG) << "socket [" << fd << "] occur other unknow event:["
                         << one_event.events
                         << "], need unregister this socket";
              delEventInLoopThread(fd);
            } else {
              if (one_event.events & EPOLLIN) {
                LOG(DEBUG) << "socket [" << fd << "] occur read event";
                Mutex::Lock lock(mutex_);
                pending_tasks_.push_back(read_cb);
              }
              if (one_event.events & EPOLLOUT) {
                LOG(DEBUG) << "socket [" << fd << "] occur write event";
                Mutex::Lock lock(mutex_);
                pending_tasks_.push_back(write_cb);
              }
            }
          }
        }
      }

      // DebugLog << "task";
      // excute tasks
      // for (size_t i = 0; i < pending_tasks_.size(); ++i) {
      // 	// DebugLog << "begin to excute task[" << i << "]";
      // 	pending_tasks_[i]();
      //   // DebugLog << "end excute tasks[" << i << "]";
      // }
      // pending_tasks_.clear();

      std::map<int, epoll_event> tmp_add;
      std::vector<int> tmp_del;

      {
        Mutex::Lock lock(mutex_);
        tmp_add.swap(pending_add_fds_);
        pending_add_fds_.clear();

        tmp_del.swap(pending_del_fds_);
        pending_del_fds_.clear();
      }
      for (auto i = tmp_add.begin(); i != tmp_add.end(); ++i) {
        // DebugLog << "fd[" << (*i).first <<"] need to add";
        addEventInLoopThread((*i).first, (*i).second);
      }
      for (auto i = tmp_del.begin(); i != tmp_del.end(); ++i) {
        // DebugLog << "fd[" << (*i) <<"] need to del";
        delEventInLoopThread((*i));
      }
    }
  }
  LOG(DEBUG) << "reactor loop end";
  looping_ = false;
}

void Reactor::stop() {
  if (!stop_flag_ && looping_) {
    stop_flag_ = true;
    wakeup();
  }
}

void Reactor::addTask(std::function<void()> task, bool is_wakeup /*=true*/) {
  {
    Mutex::Lock lock(mutex_);
    pending_tasks_.push_back(task);
  }
  if (is_wakeup) {
    wakeup();
  }
}

void Reactor::addTask(std::vector<std::function<void()>> task,
                      bool is_wakeup /* =true*/) {
  if (task.size() == 0) {
    return;
  }

  {
    Mutex::Lock lock(mutex_);
    pending_tasks_.insert(pending_tasks_.end(), task.begin(), task.end());
  }
  if (is_wakeup) {
    wakeup();
  }
}

void Reactor::addCoroutine(common::Coroutine::ptr cor,
                           bool is_wakeup /*=true*/) {
  auto func = [cor]() { common::Coroutine::Resume(cor.get()); };
  addTask(func, is_wakeup);
}

Timer* Reactor::getTimer() {
  if (init_timer_) {
    LOG(DEBUG) << "already init timer!";
  } else {
    timer_ = new Timer(this);
    timer_fd_ = timer_->getFd();
  }
  return timer_;
}

pid_t Reactor::getTid() { return tid_; }

}  // namespace net
