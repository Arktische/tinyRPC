#include "tcp_connection.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "abstract_slot.h"
#include "coroutine/coroutine_hook.h"
#include "coroutine/coroutine_pool.h"
#include "tcp_client.h"
#include "tcp_connection_time_wheel.h"
#include "tcp_server.h"
#include "timer.h"

namespace net {

TcpConnection::TcpConnection(net::TcpServer* tcp_svr, net::IOThread* io_thread,
                             int fd, int buff_size, NetAddress::ptr peer_addr)
    : m_io_thread(io_thread),
      m_fd(fd),
      m_state(Connected),
      m_connection_type(ServerConnection),
      m_peer_addr(peer_addr) {
  m_reactor = m_io_thread->getReactor();

  assert(tcp_svr != nullptr);
  m_tcp_svr = tcp_svr;

  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size);

  m_loop_cor = common::GetCoroutinePool()->getCoroutineInstanse();
  m_loop_cor->setCallBack(
      std::bind(&TcpConnection::MainServerLoopCorFunc, this));

  m_reactor->addCoroutine(m_loop_cor);

  LOG(DEBUG) << "succ create tcp connection";
}

TcpConnection::TcpConnection(net::TcpClient* tcp_cli, net::Reactor* reactor,
                             int fd, int buff_size, NetAddress::ptr peer_addr)
    : m_fd(fd),
      m_state(NotConnected),
      m_connection_type(ClientConnection),
      m_peer_addr(peer_addr) {
  assert(reactor != nullptr);
  m_reactor = reactor;

  assert(tcp_cli != nullptr);
  m_tcp_cli = tcp_cli;

  m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
  m_fd_event->setReactor(m_reactor);
  initBuffer(buff_size);

  LOG(DEBUG) << "succ create tcp connection[NotConnected]";
}

void TcpConnection::registerToTimeWheel() {
  auto cb = [](TcpConnection::ptr conn) { conn->shutdownConnection(); };
  TcpTimeWheel::TcpConnectionSlot::ptr tmp =
      std::make_shared<AbstractSlot<TcpConnection>>(shared_from_this(), cb);
  m_weak_slot = tmp;
  m_io_thread->getTimeWheel()->fresh(tmp);
}

void TcpConnection::setUpClient() { m_state = Connected; }

TcpConnection::~TcpConnection() {
  if (m_connection_type == ServerConnection) {
    common::GetCoroutinePool()->returnCoroutine(m_loop_cor->getCorId());
  }

  LOG(DEBUG) << "~TcpConnection, fd=" << m_fd;
}

void TcpConnection::initBuffer(int size) {
  // 初始化缓冲区大小
  m_write_buffer = std::make_shared<TcpBuffer>(size);
  m_read_buffer = std::make_shared<TcpBuffer>(size);
}

void TcpConnection::MainServerLoopCorFunc() {
  while (!m_stop) {
    input();

    execute();

    output();
  }
}

void TcpConnection::input() {
  if (m_state == Closed || m_state == NotConnected) {
    return;
  }
  bool read_all = false;
  bool close_flag = false;
  int count = 0;
  while (!read_all) {
    if (m_read_buffer->writeAble() == 0) {
      m_read_buffer->resizeBuffer(2 * m_read_buffer->getSize());
    }

    int read_count = m_read_buffer->writeAble();
    int write_index = m_read_buffer->writeIndex();

    LOG(DEBUG) << "m_read_buffer size="
               << m_read_buffer->getBufferVector().size()
               << "rd=" << m_read_buffer->readIndex()
               << "wd=" << m_read_buffer->writeIndex();
    int rt =
        read_hook(m_fd, &(m_read_buffer->m_buffer[write_index]), read_count);
    m_read_buffer->recycleWrite(rt);
    LOG(DEBUG) << "m_read_buffer size="
               << m_read_buffer->getBufferVector().size()
               << "rd=" << m_read_buffer->readIndex()
               << "wd=" << m_read_buffer->writeIndex();

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
            << m_peer_addr->toString() << "], fd [" << m_fd << "]";
  if (m_connection_type == ServerConnection) {
    TcpTimeWheel::TcpConnectionSlot::ptr tmp = m_weak_slot.lock();
    if (tmp) {
      m_io_thread->getTimeWheel()->fresh(tmp);
    }
  }
}

// void TcpConnection::execute() {
//   LOG(DEBUG) << "begin to do execute";
//
//   // it only server do this
//   while(m_read_buffer->readAble() > 0) {
//     TinyPbStruct pb_struct;
//     m_codec->decode(m_read_buffer.get(), &pb_struct);
//     // LOG(DEBUG) << "parse service_name=" << pb_struct.service_full_name;
//     if (!pb_struct.decode_succ) {
//       break;
//     }
//     if (m_connection_type == ServerConnection) {
//       LOG(DEBUG) << "to dispatch this package";
//       m_tcp_svr->getDispatcher()->dispatch(&pb_struct, this);
//       LOG(DEBUG) << "contine parse next package";
//     } else if (m_connection_type == ClientConnection) {
//       // TODO:
//       m_client_res_data_queue.push(pb_struct);
//     }
//
//   }
//
// }

void TcpConnection::output() {
  while (true) {
    if (m_state != Connected) {
      break;
    }

    if (m_write_buffer->readAble() == 0) {
      LOG(DEBUG) << "app buffer of fd[" << m_fd
                 << "] no data to write, to yiled this coroutine";
      break;
    }

    int total_size = m_write_buffer->readAble();
    int read_index = m_write_buffer->readIndex();
    int rt =
        write_hook(m_fd, &(m_write_buffer->m_buffer[read_index]), total_size);
    // LOG(INFO) << "write end";
    if (rt <= 0) {
      LOG(ERROR) << "write empty, error=" << strerror(errno);
    }

    LOG(DEBUG) << "succ write " << rt << " bytes";
    m_write_buffer->recycleRead(rt);
    LOG(DEBUG) << "recycle write index =" << m_write_buffer->writeIndex()
               << ", read_index =" << m_write_buffer->readIndex()
               << "readable = " << m_write_buffer->readAble();
    LOG(INFO) << "send[" << rt << "] bytes data to [" << m_peer_addr->toString()
              << "], fd [" << m_fd << "]";
    if (m_write_buffer->readAble() <= 0) {
      // LOG(INFO) << "send all data, now unregister write event on reactor and
      // yield Coroutine";
      LOG(INFO) << "send all data, now unregister write event and break";
      m_fd_event->delListenEvents(IOEvent::WRITE);
      break;
    }
  }
}

void TcpConnection::clearClient() {
  if (m_state == Closed) {
    LOG(DEBUG) << "this client has closed";
    return;
  }
  // first unregister epoll event
  m_fd_event->unregisterFromReactor();

  // stop read and write cor
  m_stop = true;

  close(m_fd_event->getFd());
  m_state = Closed;
}

void TcpConnection::shutdownConnection() {
  if (m_state == Closed || m_state == NotConnected) {
    LOG(DEBUG) << "this client has closed";
    return;
  }
  m_state = HalfClosing;
  LOG(INFO) << "shutdown conn[" << m_peer_addr->toString() << "]";
  // call sys shutdown to send FIN
  // wait client done something, client will send FIN
  // and fd occur read event but byte count is 0
  // then will call clearClient to set CLOSED
  // IOThread::MainLoopTimerFunc will delete CLOSED connection
  shutdown(m_fd_event->getFd(), SHUT_RDWR);
}

TcpBuffer* TcpConnection::getInBuffer() { return m_read_buffer.get(); }

TcpBuffer* TcpConnection::getOutBuffer() { return m_write_buffer.get(); }

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

TcpConnectionState TcpConnection::getState() const { return m_state; }

}  // namespace net
