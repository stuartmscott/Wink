// Copyright 2022-2025 Stuart Scott
#include <WinkServer/server.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace Wink::Server {

void Usage(std::string name) {
  Info() << name << std::endl;
  Info() << "\tserve [options] <directory>" << std::endl;
  Info() << "\thelp" << std::endl;
}

void Usage() { Usage("WinkServer"); }

void Help(std::string name, std::string command) {
  if (command == "serve") {
    Info() << "Starts the Wink Server to serve the given directory."
           << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << Localhost << ':'
           << ServerPort << ')' << std::endl;
    Info() << "\t-l" << std::endl;
    Info() << "\t\tThe directory to log to (default disabled)" << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\tdirectory" << std::endl;
    Info() << "\t\tThe directory containing machine binaries" << std::endl;
  } else {
    Usage();
  }
}
int ServeCommand(std::map<std::string, std::string> options,
                 std::vector<std::string> parameters) {
  Address address(Localhost, ServerPort);
  std::string log;

  if (parameters.size() == 0) {
    Error() << "Missing <directory> parameter" << std::endl;
    return -1;
  }

  // Parse Options
  for (const auto& [k, v] : options) {
    if (k == "-a") {
      std::stringstream ss(v);
      ss >> address;
    } else if (k == "-l") {
      log = v;
    } else {
      Error() << "Option " << k << ":" << v << " not supported" << std::endl;
    }
  }

  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Server s(address, mailbox, log);
  return s.Serve(parameters.at(0));
}

};  // namespace Wink::Server

int main(int argc, char** argv) {
  if (argc <= 0) {
    Wink::Server::Usage();
    return 0;
  }
  std::string name(argv[0]);
  if (argc == 1) {
    Wink::Server::Usage(name);
    return 0;
  }

  std::string command(argv[1]);

  std::map<std::string, std::string> options;
  std::vector<std::string> parameters;
  for (int i{2}; i < argc; ++i) {
    if (argv[i][0] == '-' && i + 1 < argc) {
      options.insert(
          std::make_pair(std::string(argv[i]), std::string(argv[i + 1])));
      ++i;
    } else {
      parameters.push_back(std::string(argv[i]));
    }
  }

  if (command == "serve") {
    Wink::Server::ServeCommand(options, parameters);
  } else if (command == "help") {
    if (argc < 3) {
      Wink::Server::Usage();
    } else {
      Wink::Server::Help(name, std::string(argv[2]));
    }
  } else {
    Wink::Server::Usage(name);
  }
  return 0;
}
