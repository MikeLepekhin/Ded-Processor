//
// Created by mike on 04.11.18.
//

#ifndef DED_PROCESSOR_RAM_H
#define DED_PROCESSOR_RAM_H

#include <cstdio>
#include <iostream>
#include <array>
#include "exception.h"

template<class T, size_t SIZE = 1000000>
class RAM {
 private:
  std::array<T, SIZE> memory_cells;

 public:

  T getValue(size_t address) {
    if (address > SIZE) {
      throw OutOfRangeException("incorrect memory address", __PRETTY_FUNCTION__);
    }
    return memory_cells[address];
  }

  void setValue(size_t address, const T& value) {
    if (address > SIZE) {
      throw OutOfRangeException("incorrect memory address", __PRETTY_FUNCTION__);
    }
    memory_cells[address] = value;
  }
};

#endif //DED_PROCESSOR_RAM_H
