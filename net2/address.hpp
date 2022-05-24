#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <memory>
#include <string>
namespace net2 {
class address {
 public:
  virtual auto plen() -> socklen_t* = 0;
  virtual auto len() -> socklen_t const = 0;
  virtual auto saddr() -> struct sockaddr* = 0;
};

using addr_type = std::shared_ptr<address>;

class ipv4_address final : public address,
                           std::enable_shared_from_this<ipv4_address> {
 public:
  ipv4_address(const std::string& ip, uint16_t port);
  ~ipv4_address();
  auto len() -> socklen_t const override;
  auto plen() -> socklen_t* override;
  auto saddr() -> sockaddr* override;

 private:
  sockaddr* addr_;
  socklen_t len_;
};
}  // namespace net2