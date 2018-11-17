//
// Created by mike on 03.11.18.
//

#ifndef DED_PROCESSOR_EXECUTOR_H
#define DED_PROCESSOR_EXECUTOR_H

#define NDEBUG

#include <iostream>
#include <vector>
#include <cmath>

#include "stack.h"
#include "ram.h"
#include "disassembler.h"
#include "file_buffer.h"

const size_t REGISTER_COUNT = 16;
const size_t COMMAND_COUNT = 30;
const size_t MAX_ARG_COUNT = 2;

template<class T>
struct Command {
  size_t cmd_id;
  std::string cmd_name;
  size_t arg_cnt;
  std::vector<std::pair<T, int>> args;
};

template<class T = double>
class Processor {
 private:
  T registers_[REGISTER_COUNT];
  Stack<T> stack_;
  Stack<size_t> instruction_stack_;
  RAM<T> ram_;
  FileBuffer fbuffer_;

  size_t instruction_pointer_{0};

  std::vector<Command<T>> commands;



  void parseCommand(size_t cmd_id, size_t arg_cnt, Command<T>& command) {
    command.cmd_id = cmd_id;
    command.arg_cnt = arg_cnt;
    //std::cout << "parsing of command" << cmd_id << " count of arguments: " << arg_cnt << '\n';

    for (size_t arg_id = 0; arg_id < arg_cnt; ++arg_id) {
      int cur_type = fbuffer_.readFromBuffer<int>();
      T cur_val = fbuffer_.readFromBuffer<T>();

      command.args.push_back({cur_val, cur_type});
     // std::cout << "argument " << cur_type << ' ' << cur_val << '\n';
    }
  }

  void parseAll() {

    while (!fbuffer_.done()) {
      size_t cmd_id = fbuffer_.readFromBuffer<size_t>();
      size_t arg_cnt = fbuffer_.readFromBuffer<size_t>();

      if (arg_cnt > MAX_ARG_COUNT || cmd_id > COMMAND_COUNT) {
        throw IncorrectArgumentException(std::string("incorrect cmd code or argument cnt ")
                                           + std::to_string(cmd_id) + ' ' + std::to_string(arg_cnt),
                                         __PRETTY_FUNCTION__);
      }

      commands.push_back(Command<T>());
      parseCommand(cmd_id, arg_cnt, commands.back());
    }
  }

  T getRamAddress(int arg_value) {
    int reg_num = arg_value >> 8;
    int shift = arg_value & ((1 << 8) - 1);

    return static_cast<int>(registers_[reg_num]) + shift;
  }

  T getArgumentValue(std::pair<T, int>& arg) {
    switch (arg.second) {
      case 1:
        return arg.first;
      case 2:
        return registers_[getIntValue(arg)];
      case 3:
        return ram_.getValue(getRamAddress(arg.first));
      default:
        throw IncorrectArgumentException("", __PRETTY_FUNCTION__);
    }
  }

  int getIntValue(std::pair<T, int>& arg) {
    return static_cast<int>(arg.first);
  }

  void inCmd(T& value) {
    std::cout << "# enter a value, please\n";
    std::cin >> value;
  }

  void outCmd(const T& value) const {
    std::cout << "# console out: " << value << "\n";
  }

 public:
  Processor(FILE* binary_file): fbuffer_(binary_file) {
    parseAll();
  }

  void executeCommand() {
    Command<T>& cur_command = commands[instruction_pointer_];

    switch (cur_command.cmd_id) {
      case 1:
        //std::cout << "execute 1\n";
        stack_.push(getArgumentValue(cur_command.args[0]));
        break;
      case 2:
      {
        //std::cout << "execute 2\n";
        T value = stack_.extract();

        if (cur_command.args[0].second == 2) {
          registers_[getIntValue(cur_command.args[0])] = value;
        } else if (cur_command.args[0].second == 3) {
          ram_.setValue(getRamAddress(getIntValue(cur_command.args[0])), value);
        } else {
          throw IncorrectArgumentException("", __PRETTY_FUNCTION__);
        }
        break;
      }
      case 3:
      {
        //std::cout << "execute 3\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        stack_.push(arg_a + arg_b);
        break;
      }
      case 4:
      {
        //std::cout << "execute 4\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        stack_.push(arg_a - arg_b);
        break;
      }
      case 5:
      {
        //std::cout << "execute 5\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        stack_.push(arg_a * arg_b);
        break;
      }
      case 6:
      {
        //std::cout << "execute 6\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        if (arg_b == 0.0) {
          throw DivisionByZeroException("division by zero", __PRETTY_FUNCTION__);
        }

        stack_.push(arg_a / arg_b);
        break;
      }
      case 7:
      {
        //std::cout << "execute 7\n";
        T arg_top = stack_.extract();

        stack_.push(sqrt(arg_top));
        break;
      }
      case 8:
      {
        //std::cout << "execute 8\n";
        T arg_top = stack_.top();

        stack_.push(arg_top);
        break;
      }
      case 9:
      {
        //std::cout << "execute 9\n";
        if (cur_command.args[0].second == 2) {
          inCmd(registers_[getIntValue(cur_command.args[0])]);
        } else if (cur_command.args[0].second == 3) {
          T read_value = 0.0;
          inCmd(read_value);
          ram_.setValue(getRamAddress(cur_command.args[0].first), read_value);
        } else {
          throw IncorrectArgumentException(std::to_string(cur_command.args[0].second),
                                           __PRETTY_FUNCTION__);
        }
        break;
      }
      case 10:
        //std::cout << "execute 10\n";
        outCmd(getArgumentValue(cur_command.args[0]));
        break;
      case 11:
        //std::cout << "execute 11\n";
        instruction_pointer_ = commands.size();
        return;
      case 12:
        //std::cout << "execute 12\n";
        instruction_pointer_ = cur_command.args[0].first;
        return;
      case 13:
        //std::cout << "execute 13\n";
        instruction_stack_.push(instruction_pointer_);
        instruction_pointer_ = cur_command.args[0].first;
        return;
      case 14:
      {
        //std::cout << "execute 14\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        if (arg_a == arg_b) {
          instruction_pointer_ = cur_command.args[0].first;
          return;
        }
        break;
      }
      case 15:
      {
        //std::cout << "execute 15\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        if (arg_a != arg_b) {
          instruction_pointer_ = cur_command.args[0].first;
        }
        return;
      }
      case 16:
      {
        //std::cout << "execute 16\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        if (arg_a < arg_b) {
          instruction_pointer_ = cur_command.args[0].first;
        }
        return;
      }
      case 17:
      {
        //std::cout << "execute 17\n";
        T arg_b = stack_.extract();
        T arg_a = stack_.extract();

        if (arg_a <= arg_b) {
          instruction_pointer_ = cur_command.args[0].first;
        }
        return;
      }
      case 18:
        //std::cout << "execute 18\n";
        instruction_pointer_ = instruction_stack_.extract() + 1;
        return;

      default:
        throw IncorrectArgumentException(std::string("unknown command code") +
                                           std::to_string(cur_command.cmd_id),
                                         __PRETTY_FUNCTION__);
    }
    //stack_.printStack();
    ++instruction_pointer_;
  }

  bool isDone() const {
    return instruction_pointer_ == commands.size();
  }

  void executeAll() {
    while (!isDone()) {
      executeCommand();
    }
    std::cout << "# processor: execution is finished\n";
  }
};

void execute(FILE* binary_file) {
  Processor<> processor(binary_file);

  processor.executeAll();
}

#endif //DED_PROCESSOR_EXECUTOR_H
