#ifndef TINYRPC_NET_NET_ADDRESS_H
#define TINYRPC_NET_NET_ADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <memory>



namespace net {


class NetAddress {

 public:

  typedef std::shared_ptr<NetAddress> ptr;
  
  virtual sockaddr* getSockAddr() = 0;
	
  virtual int getFamily() const = 0;

  virtual std::string toString() = 0;

  virtual socklen_t getSockLen() const = 0;

};


class IPAddress : public NetAddress {

 public:
  IPAddress(const std::string& ip, uint16_t port);

  IPAddress(uint16_t port);

  IPAddress(sockaddr_in addr);

  sockaddr* getSockAddr();

	int getFamily() const;

  socklen_t getSockLen() const;

  std::string toString();

  std::string getIp() const {
    return m_ip;
  }

  int getPort() const {
    return m_port;
  }


 private:
  
  std::string m_ip;
  uint16_t m_port;
  sockaddr_in m_addr;

};

class UnixDomainAddress : public NetAddress {
 
 public:

	UnixDomainAddress(std::string& path);

	UnixDomainAddress(sockaddr_un addr);

  sockaddr* getSockAddr();

	int getFamily() const;

  socklen_t getSockLen() const;

  std::string getPath() const {
    return m_path;
  }

  std::string toString();


 private:
	
	std::string m_path;
	sockaddr_un m_addr;

};


}


#endif

