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
  Address(const Address& address) {
    ip_ = address.ip_;
    port_ = address.port_;
  }
  Address(Address&& address) {
    ip_ = address.ip_;
    port_ = address.port_;
  }
  Address& operator=(const Address& address) {
    ip_ = address.ip_;
    port_ = address.port_;
    return *this;
  }
  Address& operator=(Address&& address) {
    ip_ = address.ip_;
    port_ = address.port_;
    return *this;
  }
  ~Address() {}

  void FromString(const std::string& address);
  std::string ToString() const;
  void ReadFrom(const struct sockaddr_in& address);
  void WriteTo(struct sockaddr_in& address) const;

  void set_ip(std::string ip) { ip_ = ip; }
  void set_port(int port) { port_ = port; }
  std::string ip() const { return ip_; }
  ushort port() const { return port_; }

  bool operator<(const Address& other) const {
    if (ip_ == other.ip_) {
      return port_ < other.port_;
    }
    return ip_ < other.ip_;
  }

  bool operator==(const Address& other) const {
    return (ip_ == other.ip_) && (port_ == other.port_);
  }

  bool operator!=(const Address& other) const { return !(*this == other); }

 private:
  std::string ip_;
  ushort port_;
};

std::istream& operator>>(std::istream& is, Address& address);
std::ostream& operator<<(std::ostream& os, const Address& address);

#endif
