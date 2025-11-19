// Copyright 2022-2025 Stuart Scott
#include <Wink/log.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::ostream& Error() {
  std::cerr << "Error: ";
  return std::cerr;
}

std::ostream& Info() { return std::cout; }

/*
Configure info and error logging output to directory.
*/
int LogToFile(const std::string& directory, const std::string& name) {
  struct stat st = {0};

  if (const auto d = directory.c_str(); stat(d, &st) == -1) {
    if (const auto result = mkdir(d, 0777); result < 0) {
      Error() << "Failed to make log directory: " << directory << ": "
              << std::strerror(errno) << std::endl;
      return -1;
    }
  }

  const auto now = std::chrono::system_clock::now();
  const auto tt = std::chrono::system_clock::to_time_t(now);
  const auto tm = *std::gmtime(&tt);

  std::ostringstream filename;
  filename << std::put_time(&tm, "%Y%m%d%H%M%S");
  filename << name;
  filename << ".log";
  std::filesystem::path filepath(directory);
  filepath /= filename.str();

  Info() << "Log: " << filepath.str() << std::endl;

  int fd = open(filepath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    Error() << "Failed to open file: " << filepath << ": "
            << std::strerror(errno) << std::endl;
    return -1;
  }

  if (const auto result = dup2(fd, 1); result < 0) {
    Error() << "Failed to redirect std::cout output to log file" << std::endl;
    close(fd);
    return -1;
  }

  if (const auto result = dup2(fd, 2); result < 0) {
    Error() << "Failed to redirect std::cerr output to log file" << std::endl;
    close(fd);
    return -1;
  }

  return close(fd);
}
