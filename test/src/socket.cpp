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

void setup_default_socket(MockSocket& socket) {
  // Set mock bind result
  {
    BindResult result;
    result.ip = kLocalhost;
    result.port = 42002;
    result.result = 0;
    socket.bindResults_.push_back(result);
  }
  // Set mock setreceivetimeout result
  {
    SetReceiveTimeoutResult result = 0;
    socket.setReceiveTimeoutResults_.push_back(result);
  }
  // Set mock send result
  {
    SendResult result = 0;
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kTestPort;
    result.result = 0;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    strcpy(result.buffer, oss.str().c_str());
    socket.receiveResults_.push_back(result);
  }
}

void assert_default_socket(MockSocket& socket, Address& parent) {
  // Bind call
  {
    ASSERT_EQ(1, socket.bindArgs_.size());
  }

  // Check socket receive timeout
  {
    ASSERT_EQ(1, socket.setReceiveTimeoutArgs_.size());
    const auto arg = socket.setReceiveTimeoutArgs_.at(0);
    ASSERT_EQ(kReplyTimeout, arg.seconds);
  }

  // Check socket send
  {
    ASSERT_EQ(4, socket.sendArgs_.size());
    // Send Started Message to Spawner
    {
      const auto arg0 = socket.sendArgs_.at(0);
      ASSERT_EQ(parent.ip(), arg0.toIP);
      ASSERT_EQ(parent.port(), arg0.toPort);
      ASSERT_EQ(std::string("started test/Test"), std::string(arg0.buffer));
      ASSERT_EQ(18, arg0.length);
    }
    // Register Machine
    {
      const auto arg1 = socket.sendArgs_.at(1);
      ASSERT_EQ(kLocalhost, arg1.toIP);
      ASSERT_EQ(kServerPort, arg1.toPort);
      std::string message(arg1.buffer);
      ASSERT_TRUE(message.starts_with("register test/Test "));
    }
    // Send Exited Message to Spawner
    {
      const auto arg2 = socket.sendArgs_.at(2);
      ASSERT_EQ(parent.ip(), arg2.toIP);
      ASSERT_EQ(parent.port(), arg2.toPort);
      ASSERT_EQ(std::string("exited test/Test"), std::string(arg2.buffer));
      ASSERT_EQ(17, arg2.length);
    }
    // Unregister Machine
    {
      const auto arg3 = socket.sendArgs_.at(3);
      ASSERT_EQ(kLocalhost, arg3.toIP);
      ASSERT_EQ(kServerPort, arg3.toPort);
      ASSERT_EQ(std::string("unregister"), std::string(arg3.buffer));
      ASSERT_EQ(11, arg3.length);
    }
  }
}
