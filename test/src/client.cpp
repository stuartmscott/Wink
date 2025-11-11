#include <Wink/client.h>
#include <Wink/log.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>

TEST(ClientTest, StartMachine) {
  MockSocket socket;

  // Set mock bind result
  {
    BindResult result;
    result.ip = kLocalhost;
    result.port = kTestPort;
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

  // Issue request
  Address address(kLocalhost, 0);
  Address destination(kTestIP, 0);
  std::vector<std::string> args;
  ASSERT_EQ(0, StartMachine(socket, address, kTestBinary, destination, args));

  // Check socket receive timeout
  {
    ASSERT_EQ(1, socket.setReceiveTimeoutArgs_.size());
    const auto arg = socket.setReceiveTimeoutArgs_.at(0);
    ASSERT_EQ(kHeartbeatTimeout, arg.seconds);
  }

  // Check socket send
  {
    ASSERT_EQ(1, socket.sendArgs_.size());
    const auto arg = socket.sendArgs_.at(0);
    ASSERT_EQ(kTestIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ(std::string("start wink.bin :0"), std::string(arg.buffer));
    ASSERT_EQ(18, arg.length);
  }

  // Check destination address
  ASSERT_EQ(kTestIP, destination.ip());
  ASSERT_EQ(kTestPort, destination.port());
}

TEST(ClientTest, StopMachine) {
  MockSocket socket;

  // Set mock send result
  {
    SendResult result = 0;
    socket.sendResults_.push_back(result);
  }

  Address address(kTestIP, kTestPort);
  ASSERT_EQ(0, StopMachine(socket, address));

  // Check socket send
  ASSERT_EQ(1, socket.sendArgs_.size());
  const auto arg = socket.sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ(std::string("stop 42424"), std::string(arg.buffer));
  ASSERT_EQ(11, arg.length);
}

TEST(ClientTest, SendMessage) {
  MockSocket socket;

  // Set mock send result
  {
    SendResult result = 0;
    socket.sendResults_.push_back(result);
  }

  Address address(kTestIP, kTestPort);
  ASSERT_EQ(0, SendMessage(socket, address, kTestMessage));

  // Check socket send
  ASSERT_EQ(1, socket.sendArgs_.size());
  const auto arg = socket.sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kTestPort, arg.toPort);
  ASSERT_EQ(kTestMessage, std::string(arg.buffer));
  ASSERT_EQ(10, arg.length);
}

TEST(ClientTest, ListMachines) {
  MockSocket socket;

  // Set mock bind result
  {
    BindResult result;
    result.ip = kLocalhost;
    result.port = kTestPort;
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
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kServerPort;
    result.result = 0;
    std::ostringstream oss;
    oss << "Port,Machine,PID\n";
    oss << kTestPort << ',' << kTestBinary << ',' << kTestPID << '\n';
    strcpy(result.buffer, oss.str().c_str());
    socket.receiveResults_.push_back(result);
  }

  // Issue request
  Address destination(kTestIP, kServerPort);
  ASSERT_EQ(0, ListMachines(socket, destination));

  // Check socket receive timeout
  {
    ASSERT_EQ(1, socket.setReceiveTimeoutArgs_.size());
    const auto arg = socket.setReceiveTimeoutArgs_.at(0);
    ASSERT_EQ(kHeartbeatTimeout, arg.seconds);
  }

  // Check socket send
  {
    ASSERT_EQ(1, socket.sendArgs_.size());
    const auto arg = socket.sendArgs_.at(0);
    ASSERT_EQ(kTestIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ(std::string("list"), std::string(arg.buffer));
    ASSERT_EQ(5, arg.length);
  }

  // Check destination address
  ASSERT_EQ(kTestIP, destination.ip());
  ASSERT_EQ(kServerPort, destination.port());
}
