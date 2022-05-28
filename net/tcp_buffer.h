#ifndef TINYRPC_NET_TCP_TCP_BUFFER_H
#define TINYRPC_NET_TCP_TCP_BUFFER_H

#include <memory>
#include <vector>

namespace net {

class TcpBuffer {
 public:
  typedef std::shared_ptr<TcpBuffer> ptr;

  TcpBuffer(int size);

  ~TcpBuffer();

  int readAble();

  int writeAble();

  int readIndex() const;

  int writeIndex() const;

  // int readFormSocket(char* buf, int size);

  void writeToBuffer(const char* buf, int size);

  void readFromBuffer(std::vector<char>& re, int size);

  void resizeBuffer(int size);

  void clearBuffer();

  int getSize();

  // const char* getBuffer();

  std::vector<char> getBufferVector();

  void recycleRead(int index);

  void recycleWrite(int index);

  void adjustBuffer();

 private:
  int read_index_{0};
  int write_index_{0};
  int size_{0};

 public:
  std::vector<char> buffer_;
};

}  // namespace net

#endif
