#include "tcp_buffer.h"

#include <string.h>
#include <unistd.h>

#include "common/log.hpp"

namespace net {

TcpBuffer::TcpBuffer(int size) { buffer_.resize(size); }

TcpBuffer::~TcpBuffer() {}

int TcpBuffer::readAble() { return write_index_ - read_index_; }

int TcpBuffer::writeAble() { return buffer_.size() - write_index_; }

int TcpBuffer::readIndex() const { return read_index_; }

int TcpBuffer::writeIndex() const { return write_index_; }

// int TcpBuffer::readFromSocket(int sockfd) {
// if (writeAble() == 0) {
// buffer_.resize(2 * size_);
// }
// int rt = read(sockfd, &buffer_[write_index_], writeAble());
// if (rt >= 0) {
// write_index_ += rt;
// }
// return rt;
// }

void TcpBuffer::resizeBuffer(int size) {
  std::vector<char> tmp(size);
  int c = std::min(size, readAble());
  memcpy(&tmp[0], &buffer_[read_index_], c);

  buffer_.swap(tmp);
  read_index_ = 0;
  write_index_ = read_index_ + c;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
  if (size > writeAble()) {
    int new_size = (int)(1.5 * (write_index_ + size));
    resizeBuffer(new_size);
  }
  memcpy(&buffer_[write_index_], buf, size);
  write_index_ += size;
}

void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
  if (readAble() == 0) {
    LOG(DEBUG) << "read buffer empty!";
    return;
  }
  int read_size = readAble() > size ? size : readAble();
  std::vector<char> tmp(read_size);

  // std::copy(read_index_, read_index_ + read_size, tmp);
  memcpy(&tmp[0], &buffer_[read_index_], read_size);
  re.swap(tmp);
  read_index_ += read_size;
  adjustBuffer();
}

void TcpBuffer::adjustBuffer() {
  if (read_index_ > static_cast<int>(buffer_.size() / 3)) {
    std::vector<char> new_buffer(buffer_.size());

    int count = readAble();
    // std::copy(&buffer_[read_index_], readAble(), &new_buffer);
    memcpy(&new_buffer[0], &buffer_[read_index_], count);

    buffer_.swap(new_buffer);
    write_index_ = count;
    read_index_ = 0;
    new_buffer.clear();
  }
}

int TcpBuffer::getSize() { return buffer_.size(); }

void TcpBuffer::clearBuffer() {
  buffer_.clear();
  read_index_ = 0;
  write_index_ = 0;
}

void TcpBuffer::recycleRead(int index) {
  int j = read_index_ + index;
  if (j >= (int)buffer_.size()) {
    LOG(ERROR) << "recycleRead error";
    return;
  }
  read_index_ = j;
  adjustBuffer();
}

void TcpBuffer::recycleWrite(int index) {
  int j = write_index_ + index;
  if (j >= (int)buffer_.size()) {
    LOG(ERROR) << "recycleWrite error";
    return;
  }
  write_index_ = j;
  adjustBuffer();
}

// const char* TcpBuffer::getBuffer() {
//   char* tmp;
//   memcpy(&tmp, &buffer_[read_index_], readAble());
//   return tmp;
// }

std::vector<char> TcpBuffer::getBufferVector() { return buffer_; }

}  // namespace net
