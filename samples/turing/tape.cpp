// Copyright 2025 Stuart Scott

#include <tape.h>

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

void Tape::Assign(std::istream& is) {
  cells_.clear();
  for (char symbol; is >> symbol;) {
    cells_.emplace_back(symbol);
  }
  pos_ = cells_.begin();
}

std::string Tape::ToString() const {
  return std::string(cells_.begin(), cells_.end());
}

char Tape::Read() const { return *pos_; }

void Tape::Write(char symbol) { *pos_ = symbol; }

void Tape::Move(char direction) {
  if (direction == 'L') {
    if (pos_ == cells_.begin()) {
      pos_ = cells_.insert(pos_, blank_);
    } else {
      --pos_;
    }
  } else if (direction == 'R') {
    ++pos_;
    if (pos_ == cells_.end()) {
      pos_ = cells_.insert(pos_, blank_);
    }
  } else if (direction != 'N') {
    throw std::invalid_argument(std::string("Unrecognized direction: ") +
                                direction);
  }
}
