// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_CONSTANTS_H_
#define TEST_INCLUDE_WINKTEST_CONSTANTS_H_

#include <Wink/constants.h>

#include <string>

namespace Wink::Test {

// Constants for Testing

constexpr size_t MaxTestPayload{20};
constexpr uint16_t TestPort{42424};
constexpr pid_t TestPID{2424};

constexpr std::string TestUnicastIP{"12.34.56.78"};
constexpr std::string TestMulticastIP{"232.34.56.78"};
constexpr std::string TestBinary{"wink.bin"};
constexpr std::string TestMessage{"test 1234"};

constexpr char TestPacket[]{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 't',
                            'e',  's',  't',  ' ',  '1',  '2',  '3',  '4'};
constexpr char TestAck[]{'\0', '\0', '\0', '\0', '\0', '\0',
                         '\0', '\0', 'a',  'c',  'k'};

constexpr size_t TestPacketLength{sizeof(TestPacket) / sizeof(TestPacket[0])};
constexpr size_t TestAckLength{sizeof(TestAck) / sizeof(TestAck[0])};

};  // namespace Wink::Test

#endif  // TEST_INCLUDE_WINKTEST_CONSTANTS_H_
