#ifndef CONSTANTS_H
#define CONSTANTS_H

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

constexpr ushort kMaxPayload = 65507;

constexpr ushort kNoTimeout = 0;          // Unlimited
constexpr ushort kReplyTimeout = 1;       // 1 second
constexpr ushort kHeartbeatTimeout = 60;  // 1 minute
constexpr ushort kReceiveTimeout = 600;   // 10 minutes

constexpr ushort kPulseInterval = 10;  // 10 seconds

constexpr ushort kServerPort = 42000;

constexpr char kLocalhost[] = "127.0.0.1";

#endif
