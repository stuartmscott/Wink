// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>

#include <cctype>
#include <cstring>
#include <string>

std::string Resolve(const std::string ip) {
  if (ip.empty() || isdigit(ip[0])) {
    // Nothing to do
    return ip;
  }

  std::string resolved(ip);
  auto c = ip.c_str();
  struct addrinfo hints, *result, *rp;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if (int status = getaddrinfo(c, NULL, &hints, &result); status != 0) {
    Error() << "Failed to get address info: " << gai_strerror(status)
            << std::endl;
  } else {
    for (rp = result; rp != NULL; rp = rp->ai_next) {
      if (rp->ai_family == AF_INET) {
        struct sockaddr_in* ipv4 =
            reinterpret_cast<struct sockaddr_in*>(rp->ai_addr);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);
        resolved.assign(ip_str);
        break;
      }
    }
    freeaddrinfo(result);
  }
  return resolved;
}

void Address::FromString(const std::string& address) {
  const auto index = address.find(':');
  switch (index) {
    case std::string::npos:
      set_ip(address);
      set_port(0);
      break;
    case 0:
      set_ip(kLocalhost);
      set_port(std::stoul(address.substr(index + 1)));
      break;
    default:
      set_ip(address.substr(0, index));
      set_port(std::stoul(address.substr(index + 1)));
      break;
  }
}

std::string Address::ToString() const {
  std::ostringstream oss;
  oss << ip_;
  oss << ':';
  oss << port_;
  return oss.str();
}

void Address::ReadFrom(const struct sockaddr_in& address) {
  set_ip(inet_ntoa(address.sin_addr));
  set_port(ntohs(address.sin_port));
}

void Address::WriteTo(struct sockaddr_in& address) const {
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  if (!ip_.empty()) {
    auto c = ip_.c_str();
    if (!isdigit(c[0])) {
      if (const auto record = gethostbyname(c); record) {
        in_addr* ia = reinterpret_cast<in_addr*>(record->h_addr);
        c = inet_ntoa(*ia);
      }
    }
    address.sin_addr.s_addr = inet_addr(c);
  }
  address.sin_port = htons(port_);
}

std::istream& operator>>(std::istream& is, Address& address) {
  std::string s;
  is >> s;
  address.FromString(s);
  return is;
}

std::ostream& operator<<(std::ostream& os, const Address& address) {
  os << address.ip() << ':' << address.port();
  return os;
}
