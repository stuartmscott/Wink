// Copyright 2025 Stuart Scott
#ifndef SAMPLES_TURING_INCLUDE_TAPE_H_
#define SAMPLES_TURING_INCLUDE_TAPE_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

class Tape {
 public:
  explicit Tape(char blank) : blank_(blank) {}
  void Assign(std::istream&);
  std::string ToString() const;
  char Read() const;
  void Write(char);
  void Move(char);

 private:
  const char blank_ = '_';
  std::vector<char> cells_;
  std::vector<char>::iterator pos_;
};

#endif  // SAMPLES_TURING_INCLUDE_TAPE_H_
