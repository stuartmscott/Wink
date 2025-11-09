#ifndef LOG_H
#define LOG_H

#include <chrono>
#include <iostream>

std::ostream& Error();
std::ostream& Info();

int LogToFile(const std::string& directory, const std::string& name);

#endif
