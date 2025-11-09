#ifndef SERVER_H
#define SERVER_H

#include <Wink/address.h>
#include <Wink/client.h>
#include <Wink/constants.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/socket.h>
#include <signal.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class Server {
 public:
  explicit Server(Address& address, Socket& socket, const std::string& log = "")
      : address_(address), socket_(socket), log_(log) {}
  Server(const Server& s) = delete;
  Server(Server&& s) = delete;
  Server& operator=(const Server& s) = delete;
  Server& operator=(Server&& s) = delete;
  ~Server() {}
  int Serve(const std::string& directory);
  int Start(const std::string& name,
            const std::vector<std::string>& parameters);
  int Stop(int port);

 private:
  Address& address_;
  Socket& socket_;
  const std::string& log_;
  // Map port number to machine file
  std::map<ushort, std::string> machines_;
  // Map port number to process identifier
  std::map<ushort, int> pids_;
};

#endif
