#include <Wink/address.h>

#include <cctype>
#include <cstring>

void Address::FromString(const std::string& address) {
  const auto index = address.find(':');
  switch (index) {
    case std::string::npos:
      ip_ = address;
      port_ = 0;
      break;
    case 0:
      ip_ = std::string(kLocalhost);
      port_ = std::stoul(address.substr(index + 1));
      break;
    default:
      ip_ = address.substr(0, index);
      port_ = std::stoul(address.substr(index + 1));
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
  ip_ = std::string(inet_ntoa(address.sin_addr));
  port_ = ntohs(address.sin_port);
}

void Address::WriteTo(struct sockaddr_in& address) const {
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  if (!ip_.empty()) {
    auto c = ip_.c_str();
    if (!isdigit(c[0])) {
      if (const auto record = gethostbyname(c); record) {
        in_addr* ia = (in_addr*)record->h_addr;
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
