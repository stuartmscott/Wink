// Copyright 2022-2025 Stuart Scott
#include <Wink/server/server.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

void Usage(std::string name) {
  Info() << name << '\n';
  Info() << "\tserve [options] <directory>\n";
  Info() << "\thelp\n";
}

void Usage() { Usage("WinkServer"); }

void Help(std::string name, std::string command) {
  if (command == "serve") {
    Info() << "Starts the Wink Server to serve the given directory.\n\n";
    Info() << "Options;";
    Info() << "\n\t-a\n\t\tThe address to bind to (default " << kLocalhost
           << ':' << kServerPort << ")\n";
    Info() << "\n\t-l\n\t\tThe directory to log to (default disabled)\n";
    Info() << "Parameters;";
    Info() << "\n\tdirectory\n\t\tThe directory containing machine files\n";
  } else {
    Usage();
  }
}

int main(int argc, char** argv) {
  if (argc <= 0) {
    Usage();
    return 0;
  }
  std::string name(argv[0]);
  if (argc == 1) {
    Usage(name);
    return 0;
  }

  std::string command(argv[1]);

  std::map<std::string, std::string> options;
  std::vector<std::string> parameters;
  for (int i = 2; i < argc; ++i) {
    if (argv[i][0] == '-' && i + 1 < argc) {
      options.insert(
          std::make_pair(std::string(argv[i]), std::string(argv[i + 1])));
      i++;
    } else {
      parameters.push_back(std::string(argv[i]));
    }
  }

  if (command == "serve") {
    Address address(kLocalhost, kServerPort);
    std::string log;

    if (parameters.size() == 0) {
      Error() << "Missing <directory> parameter\n" << std::flush;
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
        Error() << "Option " << k << ":" << v << " not supported\n"
                << std::flush;
      }
    }

    AsyncMailbox mailbox(new UDPSocket(address));
    Server s(address, mailbox, log);
    return s.Serve(parameters.at(0));
  } else if (command == "help") {
    if (argc < 3) {
      Usage();
    } else {
      Help(name, std::string(argv[2]));
    }
  } else {
    Usage(name);
  }
  return 0;
}
