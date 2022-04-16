//
// Created by tyx on 3/14/22.
//

#ifndef TINYRPC_TCP_CLIENT_HPP
#define TINYRPC_TCP_CLIENT_HPP
//#include <vector>
#include <list>
#include <string_view>

#include <common/noncopyable.hpp>
namespace lab1 {
using common::NonCopyable;
using std::list;
using std::string_view;
class TcpClient : NonCopyable {
 public:
  TcpClient();
  TcpClient(int epfd);
  ~TcpClient();

  int Connect(std::pair<string_view, uint16_t>);

 private:
  int ep_fd_;
  list<int> conn_fd_;
};
}  // namespace lab1
#endif  // TINYRPC_TCP_CLIENT_HPP
