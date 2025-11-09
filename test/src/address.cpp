#include <Wink/address.h>
#include <WinkTest/constants.h>
#include <gtest/gtest.h>

TEST(AddressTest, ReadFrom) {
  struct sockaddr_in a;
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = inet_addr(kLocalhost);
  a.sin_port = htons(kTestPort);
  Address address;
  address.ReadFrom(a);
  ASSERT_EQ(kLocalhost, address.ip());
  ASSERT_EQ(kTestPort, address.port());
}

TEST(AddressTest, WriteTo) {
  Address address(kLocalhost, kTestPort);
  struct sockaddr_in a;
  address.WriteTo(a);
  ASSERT_EQ(AF_INET, a.sin_family);
  ASSERT_EQ(inet_addr(kLocalhost), a.sin_addr.s_addr);
  ASSERT_EQ(htons(kTestPort), a.sin_port);
}

TEST(AddressTest, WriteTo_ResolveHostname) {
  Address address("localhost", kTestPort);
  struct sockaddr_in a;
  address.WriteTo(a);
  ASSERT_EQ(AF_INET, a.sin_family);
  ASSERT_EQ(inet_addr(kLocalhost), a.sin_addr.s_addr);
  ASSERT_EQ(htons(kTestPort), a.sin_port);
}

TEST(AddressTest, Stream) {
  Address a1(kLocalhost, kTestPort);
  std::ostringstream oss;
  oss << a1;

  Address a2;
  std::istringstream iss(oss.str());
  iss >> a2;

  ASSERT_EQ(kLocalhost, a2.ip());
  ASSERT_EQ(kTestPort, a2.port());
}
