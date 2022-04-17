#ifndef TINYRPC_NET_NET_ADDRESS_H
#define TINYRPC_NET_NET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <memory>

namespace core {

class Address {
 public:
  typedef std::shared_ptr<Address> ptr;

  virtual sockaddr* SockAddr() = 0;

  virtual int Family() const = 0;

  virtual std::string String() = 0;

  virtual socklen_t SockLen() const = 0;
};

class IPAddress : public Address {
 public:
  IPAddress(std::string ip, uint16_t port);

  explicit IPAddress(uint16_t port);

  explicit IPAddress(sockaddr_in addr);

  sockaddr* SockAddr() override;

  int Family() const;

  socklen_t SockLen() const;

  std::string String() override;

  std::string getIp() const { return ip_str_; }

  int getPort() const { return port_; }

 private:
  std::string ip_str_;
  uint16_t port_;
  sockaddr_in addr_in_;
};

class UnixDomainAddress : public Address {
 public:
  explicit UnixDomainAddress(std::string& path);

  explicit UnixDomainAddress(sockaddr_un addr);

  sockaddr* SockAddr() override;

  int Family() const;

  socklen_t SockLen() const;

  std::string getPath() const { return path_; }

  std::string String();

 private:
  std::string path_;
  sockaddr_un addr_un_;
};

}  // namespace net

#endif
