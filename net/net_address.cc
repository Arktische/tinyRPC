#include "net_address.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <sstream>
#include <utility>

#include "common/log.hpp"

namespace net {

IPAddress::IPAddress(std::string ip, uint16_t port)
    : ip_(std::move(ip)), port_(port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
  addr_.sin_port = htons(port_);

  LOG(DEBUG) << "create ipv4 address succ [" << toString() << "]";
}

IPAddress::IPAddress(sockaddr_in addr) : addr_(addr) {
  // if (addr_in_.sin_family != AF_INET) {
  // ErrorLog << "err family, this address is valid";
  // }
  ip_ = std::string(inet_ntoa(addr_.sin_addr));
  port_ = ntohs(addr_.sin_port);
}
IPAddress::IPAddress(uint16_t port) : port_(port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = INADDR_ANY;
  addr_.sin_port = htons(port_);

  LOG(DEBUG) << "create ipv4 address succ [" << toString() << "]";
}

int IPAddress::getFamily() const { return addr_.sin_family; }

sockaddr* IPAddress::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&addr_);
}

std::string IPAddress::toString() {
  std::stringstream ss;
  ss << ip_ << ":" << port_;
  return ss.str();
}

socklen_t IPAddress::getSockLen() const { return sizeof(addr_); }

UnixDomainAddress::UnixDomainAddress(std::string path)
    : path_(std::move(path)) {
  memset(&addr_, 0, sizeof(addr_));
  unlink(path_.c_str());
  addr_.sun_family = AF_UNIX;
  strcpy(addr_.sun_path, path_.c_str());
}
UnixDomainAddress::UnixDomainAddress(sockaddr_un addr) : addr_(addr) {
  path_ = addr_.sun_path;
}

int UnixDomainAddress::getFamily() const { return addr_.sun_family; }

sockaddr* UnixDomainAddress::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&addr_);
}

socklen_t UnixDomainAddress::getSockLen() const { return sizeof(addr_); }

std::string UnixDomainAddress::toString() { return path_; }

}  // namespace net
