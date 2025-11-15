// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_CONSTANTS_H_
#define TEST_INCLUDE_WINKTEST_CONSTANTS_H_

#include <Wink/constants.h>

#include <string>

// Constants for Testing

constexpr size_t kMaxTestPayload(20);
constexpr uint16_t kTestPort(42424);
constexpr pid_t kTestPID(2424);

constexpr std::string kTestIP("12.34.56.78");
constexpr std::string kTestBinary("wink.bin");
constexpr std::string kTestMessage("test 1234");

constexpr char kTestPacket[] = {'\0', '\0', '\0', '\0', '\0', '\0',
                                '\0', '\0', 't',  'e',  's',  't',
                                ' ',  '1',  '2',  '3',  '4'};
constexpr char kTestAck[] = {'\0', '\0', '\0', '\0', '\0', '\0',
                             '\0', '\0', 'a',  'c',  'k'};

constexpr size_t kTestPacketLength(sizeof(kTestPacket) /
                                   sizeof(kTestPacket[0]));
constexpr size_t kTestAckLength(sizeof(kTestAck) / sizeof(kTestAck[0]));

#endif  // TEST_INCLUDE_WINKTEST_CONSTANTS_H_
