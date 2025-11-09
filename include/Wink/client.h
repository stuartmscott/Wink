#ifndef CLIENT_H
#define CLIENT_H

#include <Wink/constants.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/socket.h>
#include <arpa/inet.h>

#include <chrono>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include <vector>

int StartMachine(Socket& sock, Address& addr, const std::string& name,
                 Address& dest, const std::vector<std::string> args,
                 const bool follow = false);
int StopMachine(Socket& sock, const Address& addr);
int SendMessage(Socket& sock, const Address& addr, const std::string& message);
int ListMachines(Socket& sock, const Address& server);

#endif
