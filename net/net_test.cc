#include <gtest/gtest.h>

#include "tcp_server.h"
TEST(net_test, test_tcp_server) {
  std::shared_ptr<net::NetAddress> addr(new net::IPAddress("127.0.0.1", 1080));
  net::TcpServer server(addr);
  server.start();
}
