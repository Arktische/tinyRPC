#include "address.hpp"
namespace net2 {

ipv4_address::ipv4_address(const std::string& ip, uint16_t port):len_(sizeof(sockaddr_in)) {
  auto in_addr = new sockaddr_in{AF_INET, htons(port)};
  inet_pton(AF_INET,ip.c_str(),&in_addr->sin_addr.s_addr);
  addr_ = reinterpret_cast<sockaddr*>(in_addr);
  len_ = sizeof(sockaddr_in);
}

ipv4_address::~ipv4_address() {
  delete addr_;
}

auto ipv4_address::plen() ->socklen_t* { return &len_; }

auto ipv4_address::saddr() ->struct sockaddr* {
  return addr_;
}

auto ipv4_address::len()  -> socklen_t const { return sizeof(sockaddr); }

}