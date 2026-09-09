// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <WinkTest/constants.h>
#include <gtest/gtest.h>

namespace Wink::Test {

TEST(AddressTest, ReadFrom) {
  struct sockaddr_in a;
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = inet_addr(Localhost);
  a.sin_port = htons(TestPort);
  Address address;
  address.ReadFrom(a);
  ASSERT_EQ(Localhost, address.ip());
  ASSERT_EQ(TestPort, address.port());
}

TEST(AddressTest, WriteTo) {
  Address address(Localhost, TestPort);
  struct sockaddr_in a;
  address.WriteTo(a);
  ASSERT_EQ(AF_INET, a.sin_family);
  ASSERT_EQ(inet_addr(Localhost), a.sin_addr.s_addr);
  ASSERT_EQ(htons(TestPort), a.sin_port);
}

TEST(AddressTest, WriteTo_ResolveHostname) {
  Address address("localhost", TestPort);
  struct sockaddr_in a;
  address.WriteTo(a);
  ASSERT_EQ(AF_INET, a.sin_family);
  ASSERT_EQ(inet_addr(Localhost), a.sin_addr.s_addr);
  ASSERT_EQ(htons(TestPort), a.sin_port);
}

TEST(AddressTest, IsMulticast) {
  {
    Address address("localhost", TestPort);
    ASSERT_FALSE(address.IsMulticast());
  }
  {
    Address address("192.168.1.2", TestPort);
    ASSERT_FALSE(address.IsMulticast());
  }
  {
    Address address("232.168.1.2", TestPort);
    ASSERT_TRUE(address.IsMulticast());
  }
}

TEST(AddressTest, Stream) {
  Address a1(Localhost, TestPort);
  std::ostringstream oss;
  oss << a1;

  Address a2;
  std::istringstream iss(oss.str());
  iss >> a2;

  ASSERT_EQ(Localhost, a2.ip());
  ASSERT_EQ(TestPort, a2.port());
}

};  // namespace Wink::Test
