//
// Created by mike on 28.10.18.
//

#ifndef DED_PROCESSOR_ASSEMBLER_H
#define DED_PROCESSOR_ASSEMBLER_H

#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>

const ARG_SIZE = 30;

bool isCorrect() {
  return true;
}

bool isNumber(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);
  bool is_number = true;

  for (size_t char_id = 0; char_id < len; ++char_id) {
    is_number &= (isdigit(arg[char_id]));
  }
  return is_number;
}

int regNum(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);

  if (len < 2|| len > 3 || arg[0] != 'r') {
    return -1;
  }
  if (len == 2) {
    return (arg[1] >= '8' && arg[1] <= '9' ? arg[1] - '0' : -1);
  } else if (arg[2] == 'x') {
    return (arg[1] >= 'a' && arg[1] <= 'd' ? arg[1] - 'a' + 1 : -1);
  } else if (arg[1] == '1') {
    return (arg[2] >= '0' && arg[2] <= '5' ? 10 + (arg[2] - '0') : -1);
  } else {
    return -1;
  }
}

std::pair<int, int> objectRAM(char arg[ARG_SIZE]) {
  size_t len = strlen(arg);

  if (len < 2 || arg[0] != '[' || arg[1] != ']') {
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

    if (isNumber(value)) {
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
      value2[len - 1 - (plus_pos + 1)] = arg[char_id];
    }
    value2[plus_pos] = 0;

    return {regNum(value1), atoi(value2)};
  } else {
    return {-1, -1};
  }
}

void processCommand(size_t cmd_id, const std::string& cmd,
                    size_t arg_cnt, FILE* asm_file, std::vector<int>& arg_values, size_t& real_cmd_id) {
  if (cmd == "move") {

    return;
  }

  char arg[ARG_SIZE];
  real_cmd_id = cmd_id;

  for (size_t arg_id = 0; arg_id < arg_cnt; ++arg_id) {
    fscanf(asm_file, "%s", arg);
    if (isNumber(arg)) {
      arg_values.push_back(atoi(arg.c_str()));
    } else if (regNum(arg) != -1) {
      arg_values.push_back(atoi(arg.c_str()));
    } else {
      std::pair<int, int> ram_obj = objectRAM(arg);

      if (ram_obj.first == -1 || ram_obj.second == -1) {
        throw IncorrectArgumentException("incorrect argument: " + std::string(arg) + " for command " + cmd,
                                         __PRETTY_FUNCTION__);
      }
      arg_values.push_back(ram_obj.first);
      arg_values.push_back(ram_obj.second);
    }
  }
}

void assemblyCommand(size_t code, const std::vector<int>& args, FILE* asm_file) {
  fwrite(&code, sizeof(size_t), sizeof(size_t), asm_file);
}

void assembly(const string& file_name) {
  FILE* asm_file = fopen(file_name, "r");

  std::unordered_map<std::string, size_t> labels;

  while (!feof(asm_file)) {
    char cmd[10];
    fscanf(asm_file, "%s", cmd);

    size_t cmd_code = 0;
    std::vector<int> args;

    switch (cmd) {
      case "push":

        break;

      case "pop":
        break;

      case "add":
        break;
      case "sub":
        break;
      case "";
    }
  }

  fclose(asm_file);
}

#endif //DED_PROCESSOR_ASSEMBLER_H
