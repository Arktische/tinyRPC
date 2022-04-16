#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "coroutine_hook.h"
#include "coroutine.h"
#include "net/fd_event.h"
#include "net/reactor.h"
#include "net/timer.h"
#include "common/log.hpp"

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);


HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);

// static int g_hook_enable = false;

static int g_max_timeout = 75000;

extern "C" {

void toEpoll(net::FdEvent::ptr fd_event, int events) {
	
	common::Coroutine* cur_cor = common::Coroutine::GetCurrentCoroutine() ;
	if (events & net::IOEvent::READ) {
		LOG(DEBUG) << "fd:[" << fd_event->getFd() << "], register read event to epoll";
		fd_event->setCallBack(net::IOEvent::READ, 
			[cur_cor, fd_event]() {
				common::Coroutine::Resume(cur_cor);
			}
		);
		fd_event->addListenEvents(net::IOEvent::READ);
	}
	if (events & net::IOEvent::WRITE) {
		LOG(DEBUG) << "fd:[" << fd_event->getFd() << "], register write event to epoll";
		fd_event->setCallBack(net::IOEvent::WRITE, 
			[cur_cor]() {
				common::Coroutine::Resume(cur_cor);
			}
		);
		fd_event->addListenEvents(net::IOEvent::WRITE);
	}
	// fd_event->updateToReactor();
}

ssize_t read_hook(int fd, void *buf, size_t count) {
	LOG(DEBUG) << "this is hook read";
  if (common::Coroutine::IsMainCoroutine()) {
    LOG(DEBUG) << "hook disable, call sys read func";
    return g_sys_read_fun(fd, buf, count);
  }

	net::Reactor* reactor = net::Reactor::GetReactor();
	assert(reactor != nullptr);

  net::FdEvent::ptr fd_event = net::FdEventContainer::GetFdContainer()->getFdEvent(fd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(net::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// LOG(DEBUG) << "user set nonblock, call sys func";
		// return g_sys_read_fun(fd, buf, count);
	// }

	fd_event->setNonBlock();

	// must fitst register read event on epoll
	// because reactor should always care read event when a connection sockfd was created
	// so if first call sys read, and read return success, this fucntion will not register read event and return
	// for this connection sockfd, reactor will never care read event
	toEpoll(fd_event, net::IOEvent::READ);

  ssize_t n = g_sys_read_fun(fd, buf, count);
  if (n > 0) {
    return n;
  } 

	LOG(DEBUG) << "read func to yield";
	common::Coroutine::Yield();

	// fd_event->delListenEvents(net::IOEvent::READ);
	// fd_event->updateToReactor();

	LOG(DEBUG) << "read func yield back, now to call sys read";
	return g_sys_read_fun(fd, buf, count);

}

int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	LOG(DEBUG) << "this is hook accept";
  if (common::Coroutine::IsMainCoroutine()) {
    LOG(DEBUG) << "hook disable, call sys accept func";
    return g_sys_accept_fun(sockfd, addr, addrlen);
  }
	net::Reactor* reactor = net::Reactor::GetReactor();
	assert(reactor != nullptr);

  net::FdEvent::ptr fd_event = net::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(net::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// LOG(DEBUG) << "user set nonblock, call sys func";
		// return g_sys_accept_fun(sockfd, addr, addrlen);
	// }

	fd_event->setNonBlock();

  int n = g_sys_accept_fun(sockfd, addr, addrlen);
  if (n > 0) {
    return n;
  } 

	toEpoll(fd_event, net::IOEvent::READ);
	
	LOG(DEBUG) << "accept func to yield";
	common::Coroutine::Yield();

	// fd_event->delListenEvents(net::IOEvent::READ);
	// fd_event->updateToReactor();

	LOG(DEBUG) << "accept func yield back, now to call sys accept";
	return g_sys_accept_fun(sockfd, addr, addrlen);

}

ssize_t write_hook(int fd, const void *buf, size_t count) {
	LOG(DEBUG) << "this is hook write";
  if (common::Coroutine::IsMainCoroutine()) {
    LOG(DEBUG) << "hook disable, call sys write func";
    return g_sys_write_fun(fd, buf, count);
  }
	net::Reactor* reactor = net::Reactor::GetReactor();
	assert(reactor != nullptr);

  net::FdEvent::ptr fd_event = net::FdEventContainer::GetFdContainer()->getFdEvent(fd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(net::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// LOG(DEBUG) << "user set nonblock, call sys func";
		// return g_sys_write_fun(fd, buf, count);
	// }

	fd_event->setNonBlock();

	toEpoll(fd_event, net::IOEvent::WRITE);
	
  ssize_t n = g_sys_write_fun(fd, buf, count);
  if (n > 0) {
    return n;
  }

	LOG(DEBUG) << "write func to yield";
	common::Coroutine::Yield();

	// fd_event->delListenEvents(net::IOEvent::WRITE);
	// fd_event->updateToReactor();

	LOG(DEBUG) << "write func yield back, now to call sys write";
	return g_sys_write_fun(fd, buf, count);

}

int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	LOG(DEBUG) << "this is hook connect";
  if (common::Coroutine::IsMainCoroutine()) {
    LOG(DEBUG) << "hook disable, call sys connect func";
    return g_sys_connect_fun(sockfd, addr, addrlen);
  }
	net::Reactor* reactor = net::Reactor::GetReactor();
	assert(reactor != nullptr);

  net::FdEvent::ptr fd_event = net::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(reactor);  
  }
	common::Coroutine* cur_cor = common::Coroutine::GetCurrentCoroutine();

	// if (fd_event->isNonBlock()) {
		// LOG(DEBUG) << "user set nonblock, call sys func";
    // return g_sys_connect_fun(sockfd, addr, addrlen);
	// }
	
	fd_event->setNonBlock();
  int n = g_sys_connect_fun(sockfd, addr, addrlen);
  if (n == 0) {
    LOG(DEBUG) << "direct connect succ, return";
    return n;
  } else if (errno != EINPROGRESS) {
		LOG(DEBUG) << "connect error and errno is't EINPROGRESS, errno=" << errno <<  ",error=" << strerror(errno);
    return n;
  }

	LOG(DEBUG) << "errno == EINPROGRESS";

  toEpoll(fd_event, net::IOEvent::WRITE);

	bool is_timeout = false;		// 是否超时

	// 超时函数句柄
  auto timeout_cb = [&is_timeout, cur_cor](){
		// 设置超时标志，然后唤醒协程
		is_timeout = true;
		common::Coroutine::Resume(cur_cor);
  };

  net::TimerEvent::ptr event = std::make_shared<net::TimerEvent>(g_max_timeout, false, timeout_cb);
  
  net::Timer* timer = reactor->getTimer();  
  timer->addTimerEvent(event);

  common::Coroutine::Yield();

	// write事件需要删除，因为连接成功后后面会重新监听该fd的写事件。
	fd_event->delListenEvents(net::IOEvent::WRITE); 
	// fd_event->updateToReactor();

	// 定时器也需要删除
	timer->delTimerEvent(event);

	n = g_sys_connect_fun(sockfd, addr, addrlen);
	if ((n < 0 && errno == EISCONN) || n == 0) {
		LOG(DEBUG) << "connect succ";
		return 0;
	}

	if (is_timeout) {
    LOG(ERROR) << "connect error,  timeout[ " << g_max_timeout << "ms]";
		errno = ETIMEDOUT;
	} 

	LOG(DEBUG) << "connect error and errno=" << errno <<  ", error=" << strerror(errno);
	return -1;

}

unsigned int sleep_hook(unsigned int seconds) {

  if (common::Coroutine::IsMainCoroutine()) {
    LOG(DEBUG) << "hook disable, call sys sleep func";
    return sleep(seconds);
  }

	net::Reactor* reactor = net::Reactor::GetReactor();
	assert(reactor != nullptr);

	common::Coroutine* cur_cor = common::Coroutine::GetCurrentCoroutine();
	assert(cur_cor != nullptr);

	auto timeout_cb = [cur_cor](){
		LOG(DEBUG) << "onTime, now resume cor";
		// 设置超时标志，然后唤醒协程
		common::Coroutine::Resume(cur_cor);
  };

  net::TimerEvent::ptr event = std::make_shared<net::TimerEvent>(1000 * seconds, false, timeout_cb);
  
  reactor->getTimer()->addTimerEvent(event);

  common::Coroutine::Yield();

	return 0;

}


}


namespace net {

void setMaxTimeOut (int v) {
	g_max_timeout = (v * 1000);
}

}







