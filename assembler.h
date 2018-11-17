//
// Created by mike on 28.10.18.
//

#ifndef DED_PROCESSOR_ASSEMBLER_H
#define DED_PROCESSOR_ASSEMBLER_H

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <ctype.h>

#include "exception.h"

const size_t ARG_SIZE = 30;

bool isCorrect() {
  return true;
}

bool isDigit(char ch) {
  return ch >= '0' && ch <= '9';
}

bool isIntNumber(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);
  bool is_number = true;

  for (size_t char_id = 0; char_id < len; ++char_id) {
    is_number &= (isDigit(arg[char_id]));
  }
  return is_number;
}

bool isFloatNumber(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);
  bool is_number = true;
  size_t dot_cnt = 0;

  for (size_t char_id = 0; char_id < len; ++char_id) {
    is_number &= (isDigit(arg[char_id]) || arg[char_id] == '.');
    dot_cnt += (arg[char_id] == '.');
  }
  return is_number && dot_cnt <= 1;
}

int regNum(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);

  if (len < 2|| len > 3 || arg[0] != 'r') {
    return -1;
  }
  if (len == 2) {
    return (arg[1] >= '0' && arg[1] <= '9' ? arg[1] - '0' : -1);
  } else if (arg[2] == 'x') {
    return (arg[1] >= 'a' && arg[1] <= 'e' ? arg[1] - 'a' + 1 : -1);
  } else if (arg[1] == '1') {
    return (arg[2] >= '0' && arg[2] <= '5' ? 10 + (arg[2] - '0') : -1);
  } else {
    return -1;
  }
}

std::pair<int, int> objectRAM(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);

  if (len < 2 || arg[0] != '[' || arg[len - 1] != ']') {
    return {-1, -1};
  }
  size_t plus_cnt = 0;
  int plus_pos = -1;
  for (size_t char_id = 1; char_id < len - 1; ++char_id) {
    if (arg[char_id] == '+') {
      ++plus_cnt;
      plus_pos = char_id;
    }
  }
  if (plus_cnt == 0) {
    char value[ARG_SIZE];

    for (size_t char_id = 1; char_id < len - 1; ++char_id) {
      value[char_id - 1] = arg[char_id];
    }
    value[len - 2] = 0;

    if (isIntNumber(value)) {
      return {0, atoi(value)};
    } else {
      return {regNum(value), 0};
    }
  } else if (plus_cnt == 1) {
    char value1[ARG_SIZE];
    char value2[ARG_SIZE];

    for (size_t char_id = 1; char_id < plus_pos; ++char_id) {
      value1[char_id - 1] = arg[char_id];
    }
    value1[plus_pos - 1] = 0;

    for (size_t char_id = plus_pos + 1; char_id < len - 1; ++char_id) {
      value2[char_id - (plus_pos + 1)] = arg[char_id];
    }
    value2[len - 1 - (plus_pos + 1)] = 0;


    //std::cout << "RAM obj: " << value1 << ' ' << value2 << ' ' << regNum(value1) << ' ' << atoi(value2) << '\n';

    return {regNum(value1), atoi(value2)};
  } else {
    return {-1, -1};
  }
}

void parseCommand(size_t cmd_id, const std::string& cmd, size_t arg_cnt, size_t support_mask,
                  std::vector<std::pair<double, int>>& arg_values, FILE* asm_file) {

  char arg[ARG_SIZE];

 // std::cout << cmd << ":\n";

  for (size_t arg_id = 0; arg_id < arg_cnt; ++arg_id) {
    fscanf(asm_file, "%s", arg);
  //  std::cout << "[" << std::string(arg) << "]" << '\n';
    if (isFloatNumber(arg)) {
      if (!(support_mask & 1)) {
        throw IncorrectArgumentException("numbers are not allowed as arguments of " + cmd);
      }
  //    std::cout << "is number\n";
      arg_values.push_back({atof(arg), 1});
    } else if (regNum(arg) != -1) {
      if (!(support_mask & 2)) {
        throw IncorrectArgumentException("registers are not allowed as arguments of " + cmd);
      }
  //    std::cout << "is register\n";
      arg_values.push_back({regNum(arg), 2});
    } else {
      if (!(support_mask & 4)) {
        throw IncorrectArgumentException("RAM is not allowed as argument of " + cmd);
      }

//      std::cout << "is ram\n";
      std::pair<int, int> ram_obj = objectRAM(arg);

      if (ram_obj.first == -1 || ram_obj.second == -1) {
        throw IncorrectArgumentException("incorrect argument: " + std::string(arg) + " for command " + cmd,
                                         __PRETTY_FUNCTION__);
      }
      arg_values.push_back({(ram_obj.first << 8) + ram_obj.second, 3});
    }
  }
}

void encodeCommand(size_t cmd_id, size_t arg_cnt,
                     const std::vector<std::pair<double, int>>& arg_values, FILE* binary_file) {
  fwrite(&cmd_id, sizeof(size_t), 1, binary_file);
  fwrite(&arg_cnt, sizeof(size_t), 1, binary_file);
  std::cout << "cmd " << cmd_id << ' ' << arg_cnt << '\n';
  for (size_t arg_id = 0; arg_id < arg_cnt; ++arg_id) {
    fwrite(&arg_values[arg_id].second, sizeof(int), 1, binary_file);
    fwrite(&arg_values[arg_id].first, sizeof(double), 1, binary_file);
    std::cout << arg_values[arg_id].second << ' ' << arg_values[arg_id].first << '\n';
  }
}

void assemblyCommand(size_t cmd_id, const std::string& cmd, size_t arg_cnt, size_t support_mask,
                    FILE* asm_file, FILE* binary_file) {

  std::vector<std::pair<double, int>> arg_values;

  parseCommand(cmd_id, cmd, arg_cnt, support_mask, arg_values, asm_file);
  encodeCommand(cmd_id, arg_cnt, arg_values, binary_file);
}

void removeSpaceChars(std::string& str) {
  std::string result = "";
  for (char c: str) {
    if (!isspace(c)) {
      result.push_back(c);
    }
  }
  str = result;
}

void assembly(FILE* asm_file = stdin, FILE* binary_file = stdout) {
  std::unordered_map<std::string, size_t> labels;

  while (!feof(asm_file)) {
    char cmd_buf[ARG_SIZE];
    if (fscanf(asm_file, "%s", cmd_buf) <= 0) {
      break;
    }

    std::string cmd = cmd_buf;

    removeSpaceChars(cmd);
    if (cmd == "") {
      continue;
    }

    if (cmd == "move") {
      assemblyCommand(1, "push", 1, 7, asm_file, binary_file);
      assemblyCommand(2, "pop", 1, 6, asm_file, binary_file);
      continue;
    }

    if (cmd == "push") {
      assemblyCommand(1, "push", 1, 7, asm_file, binary_file);
    } else if (cmd == "pop") {
      assemblyCommand(2, "pop", 1, 6, asm_file, binary_file);
    } else if (cmd == "add") {
      assemblyCommand(3, "add", 0, 0, asm_file, binary_file);
    } else if (cmd == "sub") {
      assemblyCommand(4, "sub", 0, 0, asm_file, binary_file);
    } else if (cmd == "mul") {
      assemblyCommand(5, "mul", 0, 0, asm_file, binary_file);
    } else if (cmd == "div") {
      assemblyCommand(6, "div", 0, 0, asm_file, binary_file);
    } else if (cmd == "sqrt") {
      assemblyCommand(7, "sqrt", 0, 0, asm_file, binary_file);
    } else if (cmd == "dup") {
      assemblyCommand(8, "dup", 0, 0, asm_file, binary_file);
    } else if (cmd == "in") {
      assemblyCommand(9, "in", 1, 6, asm_file, binary_file);
    } else if (cmd == "out") {
      assemblyCommand(10, "out", 1, 7, asm_file, binary_file);
    } else if (cmd == "end") {
      assemblyCommand(11, "end", 0, 0, asm_file, binary_file);
    } else {
      throw IncorrectArgumentException(std::string("incorrect command") + cmd);
    }
  }
  assemblyCommand(11, "end", 0, 0, asm_file, binary_file);
}

#endif //DED_PROCESSOR_ASSEMBLER_H
