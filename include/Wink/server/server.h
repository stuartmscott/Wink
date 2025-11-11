// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_SERVER_SERVER_H_
#define INCLUDE_WINK_SERVER_SERVER_H_

#include <Wink/address.h>
#include <Wink/client.h>
#include <Wink/constants.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
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
  explicit Server(Address& address, Mailbox& mailbox,
                  const std::string& log = "")
      : address_(address), mailbox_(mailbox), log_(log) {}
  Server(const Server& s) = delete;
  Server(Server&& s) = delete;
  Server& operator=(const Server& s) = delete;
  Server& operator=(Server&& s) = delete;
  ~Server() {}
  int Serve(const std::string& directory);
  int Start(const std::string& name,
            const std::vector<std::string>& parameters);
  int Stop(int port);
  std::string List();
  void Shutdown();

 private:
  Address& address_;
  Mailbox& mailbox_;
  const std::string& log_;
  std::atomic_bool running_;
  // Map port number to machine file
  std::map<ushort, std::string> machines_;
  // Map port number to process identifier
  std::map<ushort, int> pids_;
};

#endif  // INCLUDE_WINK_SERVER_SERVER_H_
