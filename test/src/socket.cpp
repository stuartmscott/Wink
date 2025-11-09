#include <Wink/log.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>

#include <cstring>

int MockSocket::Bind(Address& address) {
  const auto index = bindArgs_.size();
  BindArgs args{address.ip(), address.port()};
  bindArgs_.push_back(args);
  if (index >= bindResults_.size()) {
    Error() << "Unexpected call to Bind\n" << std::flush;
  }
  const auto result = bindResults_.at(index);
  address.set_ip(result.ip);
  address.set_port(result.port);
  return result.result;
}

int MockSocket::SetReceiveTimeout(const int seconds) {
  const auto index = setReceiveTimeoutArgs_.size();
  SetReceiveTimeoutArgs args{seconds};
  setReceiveTimeoutArgs_.push_back(args);
  if (index >= setReceiveTimeoutResults_.size()) {
    Error() << "Unexpected call to SetReceiveTimeout\n" << std::flush;
  }
  return setReceiveTimeoutResults_.at(index);
}

int MockSocket::Receive(Address& from, char* buffer, const int length,
                        const int flags) {
  const auto index = receiveArgs_.size();
  ReceiveArgs args;
  args.length = length;
  args.flags = flags;
  receiveArgs_.push_back(args);
  if (index >= receiveResults_.size()) {
    Error() << "Unexpected call to Receive\n" << std::flush;
  }
  const auto result = receiveResults_.at(index);
  from.set_ip(result.fromIP);
  from.set_port(result.fromPort);
  strcpy(buffer, result.buffer);
  return result.result;
}

int MockSocket::Send(const Address& to, const char* buffer, const int length,
                     const int flags) {
  const auto index = sendArgs_.size();
  SendArgs args;
  args.toIP = to.ip();
  args.toPort = to.port();
  args.length = length;
  args.flags = flags;
  strcpy(args.buffer, buffer);
  sendArgs_.push_back(args);
  if (index >= sendResults_.size()) {
    Error() << "Unexpected call to Send\n" << std::flush;
  }
  return sendResults_.at(index);
}
