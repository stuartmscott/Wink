// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_CONSTANTS_H_
#define INCLUDE_WINK_CONSTANTS_H_

#include <chrono>
#include <cstdint>

constexpr size_t kMaxUDPPayload = 65507;

constexpr uint8_t kMaxRetries = 5;

constexpr std::chrono::seconds kNoTimeout(0);  // Unlimited
constexpr std::chrono::seconds kSendTimeout(1);
constexpr std::chrono::seconds kReceiveTimeout(3);

constexpr std::chrono::seconds kHeartbeatTimeout(60);
constexpr std::chrono::seconds kPulseInterval(10);

constexpr uint16_t kServerPort = 42000;

constexpr char kLocalhost[] = "127.0.0.1";

#endif  // INCLUDE_WINK_CONSTANTS_H_
