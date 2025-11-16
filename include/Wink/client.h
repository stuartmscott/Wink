// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_CLIENT_H_
#define INCLUDE_WINK_CLIENT_H_

#include <Wink/constants.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
#include <arpa/inet.h>

#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include <vector>

int StartMachine(Mailbox& mailbox, const Address address,
                 const std::string name, Address& destination,
                 const std::vector<std::string> args,
                 const bool follow = false);
int StopMachine(Mailbox& mailbox, const Address address);
void SendMessage(Mailbox& mailbox, const Address to, const std::string message);
void SendMessages(Mailbox& mailbox, const Address to,
                  const std::vector<std::string> message);
bool ReceiveMessage(Mailbox& mailbox, Address& from, std::string& message);
int ListMachines(Mailbox& mailbox, const Address server);

#endif  // INCLUDE_WINK_CLIENT_H_
