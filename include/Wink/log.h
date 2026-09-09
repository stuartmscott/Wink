// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_LOG_H_
#define INCLUDE_WINK_LOG_H_

#include <chrono>
#include <iostream>
#include <string>

namespace Wink {

std::ostream& Error();
std::ostream& Info();

int LogToFile(const std::string& directory, const std::string& name);

};  // namespace Wink

#endif  // INCLUDE_WINK_LOG_H_
