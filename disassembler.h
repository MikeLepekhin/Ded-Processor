#ifndef DED_PROCESSOR_DISASSEMBLER_H
#define DED_PROCESSOR_DISASSEMBLER_H

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <algorithm>
#include <unordered_map>

#include "file_buffer.h"
#include "exception.h"
#include "common_classes.h"

const int MAX_ARG_CNT = 10;

void readCommand(size_t cmd_id, const std::string& cmd,
                 size_t arg_cnt, size_t support_mask,
                 std::vector<std::pair<double, int>>& arg_values, FileBuffer& fbuffer) {

  size_t arg_cnt_file = fbuffer.readFromBuffer<size_t>();
  std::cout << "argument cnt in file: " << arg_cnt_file << '\n';

  if (arg_cnt == 0) {
    return;
  }

  for (size_t arg_id = 0; arg_id < arg_cnt; ++arg_id) {
    int cur_type = fbuffer.readFromBuffer<int>();
    double cur_val = fbuffer.readFromBuffer<double>();

    arg_values.push_back({cur_val, cur_type});
    std::cout << cur_val << ' ' << cur_type << '\n';
  }
  std::cout << "real arg number:" << ' ' << arg_cnt << " arguments\n";
}

std::string getRegisterById(size_t id) {
  if (id >= 1 && id <= 5) {
    return std::string("r") + char('a' + id - 1) + char('x');
  }
  if (id >= 6 && id <= 15) {
    return std::string("r") + std::to_string(id);
  }
  throw IncorrectArgumentException("there is no a register with such id: " + std::to_string(id));
}

std::string getRamObject(int encoded) {
  int reg_num = (encoded >> 8);
  int shift = encoded & ((1 << 8) - 1);

  std::string reg = reg_num > 0 ? getRegisterById(reg_num) : "";
  std::string shift_str = shift > 0 ? std::to_string(shift) : "";

  return std::string("[") + reg + (reg_num > 0 && shift > 0 ? "+" : "") + shift_str + "]";
}

void decodeArgument(const std::pair<double, int>& arg, FILE* decode_file) {
  if (arg.second < 1 || arg.second > 3) {
    throw IncorrectArgumentException("Argument type should be a number from 1 to 3");
  }
  switch (arg.second) {
    case 1:
      fprintf(decode_file, "%lf", arg.first);
      break;
    case 2:
      fprintf(decode_file, "%s", getRegisterById(static_cast<size_t>(arg.first)).c_str());
      break;
    case 3:
    std::cout << "RAM object: " << int(arg.first) << '\n';
      fprintf(decode_file, "%s", getRamObject(static_cast<int>(arg.first)).c_str());
      break;
    default:
      throw IncorrectArgumentException("Argument type should be a number from 1 to 3");
  }
}

void decodeCommand(const Command<double>& command, size_t cmd_order,
                   const std::set<size_t>& jumps_to, FILE* decode_file) {
  if (jumps_to.find(cmd_order) != jumps_to.end()) {
    fprintf(decode_file, "\n:label_cmd_");
    fprintf(decode_file, "%zu\n", cmd_order);
  }

  fprintf(decode_file, "%s", command.cmd_name.c_str());

  for (size_t arg_id = 0; arg_id < command.arg_cnt; ++arg_id) {
    fprintf(decode_file, " ");
    if (isJump(command.cmd_name)) {
      fprintf(decode_file, "label_cmd_");
      fprintf(decode_file, "%zu\n", static_cast<size_t>(command.args[arg_id].first));
    } else {
      decodeArgument(command.args[arg_id], decode_file);
    }
  }
  fprintf(decode_file, "\n");
}

void disassemblyCommand(size_t cmd_id, const char* cmd, size_t arg_cnt, size_t support_mask,
                     FileBuffer& fbuffer, std::vector<Command<double>>& commands,
                     std::set<size_t>& jumps_to, FILE* decode_file) {

  std::vector<std::pair<double, int>> args;

  readCommand(cmd_id, cmd, arg_cnt, support_mask, args, fbuffer);
  if (isJump(std::string(cmd))) {
    jumps_to.insert(static_cast<size_t>(args[0].first));
  }
  commands.push_back(Command<double>{cmd_id, std::string(cmd), arg_cnt, args});
  //decodeCommand(cmd, arg_cnt, arg_values, decode_file);
}

void disassembly(FILE* binary_file = stdin, FILE* decode_file = stdout) {
  FileBuffer fbuffer(binary_file);

  std::set<size_t> jumps_to;
  std::vector<Command<double>> commands;

  while (!fbuffer.done()) {
    size_t cmd_id = fbuffer.readFromBuffer<size_t>();

    if (cmd_id == 1) {
      disassemblyCommand(cmd_id, "push", 1, 7, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 2) {
      disassemblyCommand(cmd_id, "pop", 1, 6, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 3) {
      disassemblyCommand(3, "add", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 4) {
      disassemblyCommand(4, "sub", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 5) {
      disassemblyCommand(5, "mul", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 6) {
      disassemblyCommand(6, "div", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 7) {
      disassemblyCommand(7, "sqrt", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 8) {
      disassemblyCommand(8, "dup", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 9) {
      disassemblyCommand(9, "in", 1, 6, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 10) {
      disassemblyCommand(10, "out", 1, 7, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 11) {
      disassemblyCommand(11, "end", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 12) {
      disassemblyCommand(12, "jmp", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 13) {
      disassemblyCommand(13, "call", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 14) {
      disassemblyCommand(14, "je", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 15) {
      disassemblyCommand(15, "jne", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 16) {
      disassemblyCommand(16, "jl", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 17) {
      disassemblyCommand(17, "jle", 1, 1, fbuffer, commands, jumps_to, decode_file);
    } else if (cmd_id == 18) {
      disassemblyCommand(18, "ret", 0, 0, fbuffer, commands, jumps_to, decode_file);
    } else {
      throw IncorrectArgumentException(std::string("incorrect command id ") + std::to_string(cmd_id));
    }
  }

  for (size_t cmd_order = 0; cmd_order < commands.size(); ++cmd_order) {
    decodeCommand(commands[cmd_order], cmd_order, jumps_to, decode_file);
  }
}

#endif //DED_PROCESSOR_DISASSEMBLER_H
