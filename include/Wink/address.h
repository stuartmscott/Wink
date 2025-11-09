#ifndef ADDRESS_H
#define ADDRESS_H

#include <Wink/constants.h>
#include <Wink/log.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <string>

class Address {
 public:
  Address() : ip_(kLocalhost), port_(0) {}
  Address(std::string address) { FromString(address); }
  Address(std::string ip, ushort port) : ip_(ip), port_(port) {}
  Address(const Address& addr) = delete;
  Address(Address&& addr) = delete;
  ~Address() {}
  void FromString(const std::string& addr);
  std::string ToString() const;
  void ReadFrom(const struct sockaddr_in& addr);
  void WriteTo(struct sockaddr_in& addr) const;
  void set_ip(std::string ip) { ip_ = ip; }
  void set_port(int port) { port_ = port; }
  std::string ip() const { return ip_; }
  ushort port() const { return port_; }

 private:
  std::string ip_;
  ushort port_;
};

std::istream& operator>>(std::istream& is, Address& addr);
std::ostream& operator<<(std::ostream& os, const Address& addr);

#endif
