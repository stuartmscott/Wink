// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_ADDRESS_H_
#define INCLUDE_WINK_ADDRESS_H_

#include <Wink/constants.h>
#include <Wink/log.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <string>

std::string Resolve(const std::string ip);

class Address {
 public:
  Address() {
    set_ip(kLocalhost);
    set_port(0);
  }
  explicit Address(std::string address) { FromString(address); }
  Address(std::string ip, uint16_t port) {
    set_ip(ip);
    set_port(port);
  }
  Address(const Address& address) {
    set_ip(address.ip_);
    set_port(address.port_);
  }
  Address(Address&& address) {
    set_ip(address.ip_);
    set_port(address.port_);
  }
  Address& operator=(const Address& address) {
    set_ip(address.ip_);
    set_port(address.port_);
    return *this;
  }
  Address& operator=(Address&& address) {
    set_ip(address.ip_);
    set_port(address.port_);
    return *this;
  }
  ~Address() {}

  void FromString(const std::string& address);
  std::string ToString() const;
  void ReadFrom(const struct sockaddr_in& address);
  void WriteTo(struct sockaddr_in& address) const;
  uint32_t ToInetAddr() const;
  bool IsMulticast() const;

  void set_ip(std::string ip) {
    ip_ = ip;
    resolved_ = Resolve(ip_);
  }
  void set_port(uint16_t port) { port_ = port; }
  std::string ip() const { return ip_; }
  uint16_t port() const { return port_; }

  bool operator<(const Address& other) const {
    if (resolved_ == other.resolved_) {
      return port_ < other.port_;
    }
    return resolved_ < other.resolved_;
  }

  bool operator==(const Address& other) const {
    if (port_ == other.port_) {
      return true;
    }
    if (ip_ == other.ip_) {
      return true;
    }
    return resolved_ == other.resolved_;
  }

  bool operator!=(const Address& other) const { return !(*this == other); }

 private:
  std::string ip_ = kLocalhost;
  std::string resolved_ = kLocalhost;
  uint16_t port_ = 0;
};

std::istream& operator>>(std::istream& is, Address& address);
std::ostream& operator<<(std::ostream& os, const Address& address);

#endif  // INCLUDE_WINK_ADDRESS_H_
