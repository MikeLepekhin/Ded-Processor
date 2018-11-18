//
// Created by mike on 18.11.18.
//

#ifndef DED_PROCESSOR_COMMON_CLASSES_H
#define DED_PROCESSOR_COMMON_CLASSES_H

#include <iostream>
#include <vector>
#include <string>


class SmartFile {
 private:
  FILE* file_{nullptr};

  void release() {
    if (file_ != nullptr) {
      fclose(file_);
    }
    file_ = nullptr;
  }

 public:
  SmartFile() {}

  SmartFile(const char* filename, const char* mode = "r") {
    file_ = fopen(filename, mode);
  }

  SmartFile(FILE* file) {
    file_ = file;
  }

  FILE* getFile() const {
    return file_;
  }

  void setFile(const char* filename, const char* mode = "r") {
    release();
    file_ = fopen(filename, mode);
  }

  ~SmartFile() {
    release();
  }
};

template<class T>
struct Command {
  size_t cmd_id;
  std::string cmd_name;
  size_t arg_cnt;
  std::vector<std::pair<T, int>> args;
};

#endif //DED_PROCESSOR_COMMON_CLASSES_H
