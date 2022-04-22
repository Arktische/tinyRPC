#include "tcp_connection.h"

#include <common/coroutine/coroutine_pool.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <utility>

#include "coroutine_hook.h"
#include "tcp_client.h"
#include "tcp_conn_timer.h"
#include "tcp_server.h"
#include "timer.h"

namespace net {

TcpConnection::TcpConnection(net::TcpServer* tcp_svr, net::IOThread* io_thread,
                             int fd, int buff_size, NetAddress::ptr peer_addr)
    : io_thread_(io_thread),
      fd_(fd),
      state_(Connected),
      conn_type_(ServerConnection),
      peer_addr_(std::move(peer_addr)) {
  reactor_ = io_thread_->getReactor();

  assert(tcp_svr != nullptr);
  tcp_svr_ = tcp_svr;

  fd_event_ = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  fd_event_->setReactor(reactor_);
  initBuffer(buff_size);

  loop_coroutine_ = common::GetCoroutinePool()->getCoroutineInstanse();
  loop_coroutine_->setCallBack([this] { MainServerLoopCorFunc(); });

  reactor_->addCoroutine(loop_coroutine_);

  LOG(DEBUG) << "succ create tcp connection";
}

TcpConnection::TcpConnection(net::TcpClient* tcp_cli, net::Reactor* reactor,
                             int fd, int buff_size, NetAddress::ptr peer_addr)
    : fd_(fd),
      state_(NotConnected),
      conn_type_(ClientConnection),
      peer_addr_(std::move(peer_addr)) {
  assert(reactor != nullptr);
  reactor_ = reactor;

  assert(tcp_cli != nullptr);
  tcp_cli_ = tcp_cli;

  fd_event_ = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  fd_event_->setReactor(reactor_);
  initBuffer(buff_size);

  LOG(DEBUG) << "succ create tcp connection[NotConnected]";
}

void TcpConnection::registerToTimeWheel() {
  auto cb = [](const TcpConnection::ptr& conn) { conn->shutdownConnection(); };
  TcpTimeWheel::TcpConnectionSlot::ptr tmp =
      std::make_shared<AbstractSlot<TcpConnection>>(shared_from_this(), cb);
  weak_slot_ = tmp;
  io_thread_->getTimeWheel()->fresh(tmp);
}

void TcpConnection::setUpClient() { state_ = Connected; }

TcpConnection::~TcpConnection() {
  if (conn_type_ == ServerConnection) {
    common::GetCoroutinePool()->returnCoroutine(loop_coroutine_->getCorId());
  }

  LOG(DEBUG) << "~TcpConnection, fd=" << fd_;
}

void TcpConnection::initBuffer(int size) {
  // 初始化缓冲区大小
  write_buffer_ = std::make_shared<TcpBuffer>(size);
  read_buffer_ = std::make_shared<TcpBuffer>(size);
}

void TcpConnection::MainServerLoopCorFunc() {
  while (!stop_) {
    input();
    rcb_(shared_from_this());
    // execute();
    wcb_(shared_from_this());
    output();
  }
}

void TcpConnection::input() {
  if (state_ == Closed || state_ == NotConnected) {
    return;
  }
  bool read_all = false;
  bool close_flag = false;
  int count = 0;
  while (!read_all) {
    if (read_buffer_->writeAble() == 0) {
      read_buffer_->resizeBuffer(2 * read_buffer_->getSize());
    }

    int read_count = read_buffer_->writeAble();
    int write_index = read_buffer_->writeIndex();

    LOG(DEBUG) << "read_buffer_ size=" << read_buffer_->getBufferVector().size()
               << "rd=" << read_buffer_->readIndex()
               << "wd=" << read_buffer_->writeIndex();
    int rt = read_hook(fd_, &(read_buffer_->buffer_[write_index]), read_count);
    read_buffer_->recycleWrite(rt);
    LOG(DEBUG) << "read_buffer_ size=" << read_buffer_->getBufferVector().size()
               << "rd=" << read_buffer_->readIndex()
               << "wd=" << read_buffer_->writeIndex();

    LOG(DEBUG) << "read data back";
    count += rt;
    if (rt <= 0) {
      LOG(DEBUG) << "rt <= 0";
      LOG(ERROR) << "read empty while occur read event, because of peer close, "
                    "sys error="
                 << strerror(errno) << ", now to clear tcp connection";
      clearClient();
      // this cor can destroy
      close_flag = true;
      break;
      read_all = true;
    } else {
      if (rt == read_count) {
        LOG(DEBUG) << "read_count == rt";
        // is is possible read more data, should continue read
        continue;
      } else if (rt < read_count) {
        LOG(DEBUG) << "read_count > rt";
        // read all data in socket buffer, skip out loop
        read_all = true;
        break;
      }
    }
  }
  if (close_flag) {
    return;
  }
  if (!read_all) {
    LOG(ERROR) << "not read all data in socket buffer";
  }
  LOG(INFO) << "recv [" << count << "] bytes data from ["
            << peer_addr_->toString() << "], fd [" << fd_ << "]";
  if (conn_type_ == ServerConnection) {
    TcpTimeWheel::TcpConnectionSlot::ptr tmp = weak_slot_.lock();
    if (tmp) {
      io_thread_->getTimeWheel()->fresh(tmp);
    }
  }
}

// void TcpConnection::execute() {
//   LOG(DEBUG) << "begin to do execute";
//
//   // it only server do this
//   while(read_buffer_->readAble() > 0) {
//     TinyPbStruct pb_struct;
//     m_codec->decode(read_buffer_.get(), &pb_struct);
//     // LOG(DEBUG) << "parse service_name=" << pb_struct.service_full_name;
//     if (!pb_struct.decode_succ) {
//       break;
//     }
//     if (conn_type_ == ServerConnection) {
//       LOG(DEBUG) << "to dispatch this package";
//       tcp_svr_->getDispatcher()->dispatch(&pb_struct, this);
//       LOG(DEBUG) << "contine parse next package";
//     } else if (conn_type_ == ClientConnection) {
//       // TODO:
//       m_client_res_data_queue.push(pb_struct);
//     }
//
//   }
//
// }

void TcpConnection::output() {
  while (true) {
    if (state_ != Connected) {
      break;
    }

    if (write_buffer_->readAble() == 0) {
      LOG(DEBUG) << "app buffer of fd[" << fd_
                 << "] no data to write, to yiled this coroutine";
      break;
    }

    int total_size = write_buffer_->readAble();
    int read_index = write_buffer_->readIndex();
    int rt = write_hook(fd_, &(write_buffer_->buffer_[read_index]), total_size);
    // LOG(INFO) << "write end";
    if (rt <= 0) {
      LOG(ERROR) << "write empty, error=" << strerror(errno);
    }

    LOG(DEBUG) << "succ write " << rt << " bytes";
    write_buffer_->recycleRead(rt);
    LOG(DEBUG) << "recycle write index =" << write_buffer_->writeIndex()
               << ", read_index =" << write_buffer_->readIndex()
               << "readable = " << write_buffer_->readAble();
    LOG(INFO) << "send[" << rt << "] bytes data to [" << peer_addr_->toString()
              << "], fd [" << fd_ << "]";
    if (write_buffer_->readAble() <= 0) {
      // LOG(INFO) << "send all data, now unregister write event on reactor and
      // yield Coroutine";
      LOG(INFO) << "send all data, now unregister write event and break";
      fd_event_->delListenEvents(IOEvent::WRITE);
      break;
    }
  }
}

void TcpConnection::clearClient() {
  if (state_ == Closed) {
    LOG(DEBUG) << "this client has closed";
    return;
  }
  // first unregister epoll event
  fd_event_->unregisterFromReactor();

  // stop read and write cor
  stop_ = true;

  close(fd_event_->getFd());
  state_ = Closed;
}

void TcpConnection::shutdownConnection() {
  if (state_ == Closed || state_ == NotConnected) {
    LOG(DEBUG) << "this client has closed";
    return;
  }
  state_ = HalfClosing;
  LOG(INFO) << "shutdown conn[" << peer_addr_->toString() << "]";
  // call sys shutdown to send FIN
  // wait client done something, client will send FIN
  // and fd occur read event but byte count is 0
  // then will call clearClient to set CLOSED
  // IOThread::MainLoopTimerFunc will delete CLOSED connection
  shutdown(fd_event_->getFd(), SHUT_RDWR);
}

TcpBuffer* TcpConnection::getInBuffer() { return read_buffer_.get(); }

TcpBuffer* TcpConnection::getOutBuffer() { return write_buffer_.get(); }

// bool TcpConnection::getResPackageData(TinyPbStruct& pb_struct) {
//   if (!m_client_res_data_queue.empty()) {
//     LOG(DEBUG) << "return a resdata";
//     pb_struct = m_client_res_data_queue.front();
//     m_client_res_data_queue.pop();
//     return true;
//   }
//   return false;
//
// }

// TinyPbCodeC* TcpConnection::getCodec() const {
//   return m_codec.get();
// }

TcpConnectionState TcpConnection::getState() const { return state_; }
void TcpConnection::execute() {}

}  // namespace net
