#include "addr.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <utility>

#include "common/log.hpp"
//#include "../comm/log.h"

namespace core {

IPAddress::IPAddress(std::string ip, uint16_t port)
    : ip_str_(std::move(ip)), port_(port) {
  memset(&addr_in_, 0, sizeof(addr_in_));
  addr_in_.sin_family = AF_INET;
  addr_in_.sin_addr.s_addr = inet_addr(ip_str_.c_str());
  addr_in_.sin_port = htons(port_);

  LOG(DEBUG) << "create ipv4 address succ [" << String() << "]";
}

IPAddress::IPAddress(sockaddr_in addr) : addr_in_(addr) {
  // if (addr_in_.sin_family != AF_INET) {
  // ErrorLog << "err family, this address is valid";
  // }
  ip_str_ = std::string(inet_ntoa(addr_in_.sin_addr));
  port_ = ntohs(addr_in_.sin_port);
}
IPAddress::IPAddress(uint16_t port) : port_(port) {
  memset(&addr_in_, 0, sizeof(addr_in_));
  addr_in_.sin_family = AF_INET;
  addr_in_.sin_addr.s_addr = INADDR_ANY;
  addr_in_.sin_port = htons(port_);

  LOG(DEBUG) << "create ipv4 address succ [" << String() << "]";
}

int IPAddress::Family() const { return addr_in_.sin_family; }

sockaddr* IPAddress::SockAddr() {
  return reinterpret_cast<sockaddr*>(&addr_in_);
}

std::string IPAddress::String() {
  std::stringstream ss;
  ss << ip_str_ << ":" << port_;
  return ss.str();
}

socklen_t IPAddress::SockLen() const { return sizeof(addr_in_); }

UnixDomainAddress::UnixDomainAddress(std::string& path) : path_(path) {
  memset(&addr_un_, 0, sizeof(addr_un_));
  unlink(path_.c_str());
  addr_un_.sun_family = AF_UNIX;
  strcpy(addr_un_.sun_path, path_.c_str());
}
UnixDomainAddress::UnixDomainAddress(sockaddr_un addr) : addr_un_(addr) {
  path_ = addr_un_.sun_path;
}

int UnixDomainAddress::Family() const { return addr_un_.sun_family; }

sockaddr* UnixDomainAddress::SockAddr() {
  return reinterpret_cast<sockaddr*>(&addr_un_);
}

socklen_t UnixDomainAddress::SockLen() const { return sizeof(addr_un_); }

std::string UnixDomainAddress::String() { return path_; }

}  // namespace net
