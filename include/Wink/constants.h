// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_CONSTANTS_H_
#define INCLUDE_WINK_CONSTANTS_H_

#include <chrono>
#include <cstdint>

namespace Wink {

constexpr size_t MaxUDPPayload{65507};

constexpr uint8_t MaxRetries{10};

constexpr std::chrono::microseconds NoTimeout{0};            // Unlimited
constexpr std::chrono::microseconds SendTimeout{10000};      // 10ms
constexpr std::chrono::microseconds ReceiveTimeout{100000};  // 100ms

constexpr std::chrono::seconds HeartbeatTimeout{60};
constexpr std::chrono::seconds PulseInterval{10};

constexpr uint16_t ServerPort{42000};

constexpr char Localhost[]{"127.0.0.1"};

};  // namespace Wink

#endif  // INCLUDE_WINK_CONSTANTS_H_
