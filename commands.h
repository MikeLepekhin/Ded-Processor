//
// Created by mike on 18.11.18.
//

COMMAND(1, "push", 1, 7,\
  stack_.push(getArgumentValue(cur_command.args[0]));\
)
COMMAND(2, "pop", 1, 6, \
  T value = stack_.extract();\
\
  if (cur_command.args[0].second == 2) {\
    registers_[getIntValue(cur_command.args[0])] = value;\
  } else if (cur_command.args[0].second == 3) {\
    ram_.setValue(getRamAddress(getIntValue(cur_command.args[0])), value);\
  } else {\
    throw IncorrectArgumentException("", __PRETTY_FUNCTION__);\
  }\
)

COMMAND(3, "add", 0, 0,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  stack_.push(arg_a + arg_b);\
)
COMMAND(4, "sub", 0, 0,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  stack_.push(arg_a - arg_b);\
)
COMMAND(5, "mul", 0, 0,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  stack_.push(arg_a * arg_b);\
)
COMMAND(6, "div", 0, 0,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  if (arg_b == 0.0) {\
    throw DivisionByZeroException("division by zero", __PRETTY_FUNCTION__);\
  }\
  stack_.push(arg_a / arg_b);\
)
COMMAND(7, "sqrt", 0, 0,\
  T arg_top = stack_.extract();\
\
  stack_.push(sqrt(arg_top));\
)
COMMAND(8, "dup", 0, 0,\
  T arg_top = stack_.top();\
\
  stack_.push(arg_top);\
)
COMMAND(9, "in", 1, 6,\
  if (cur_command.args[0].second == 2) {\
    inCmd(registers_[getIntValue(cur_command.args[0])]);\
  } else if (cur_command.args[0].second == 3) {\
    T read_value = 0.0;\
    inCmd(read_value);\
    ram_.setValue(getRamAddress(cur_command.args[0].first), read_value);\
  } else {\
    throw IncorrectArgumentException(std::to_string(cur_command.args[0].second),\
                                    __PRETTY_FUNCTION__);\
  }\
)
COMMAND(10, "out", 1, 7,\
  outCmd(getArgumentValue(cur_command.args[0]));\
)
COMMAND(11, "end", 0, 0,\
  instruction_pointer_ = commands.size();\
  return;\
)
COMMAND(12, "jmp", 1, 1,\
  instruction_pointer_ = cur_command.args[0].first;\
  return;\
)
COMMAND(13, "call", 1, 1,\
  instruction_stack_.push(instruction_pointer_);\
  instruction_pointer_ = cur_command.args[0].first;\
  return;\
)
COMMAND(14, "je", 1, 1,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  if (arg_a == arg_b) {\
    instruction_pointer_ = cur_command.args[0].first;\
    return;\
  }\
)
COMMAND(15, "jne", 1, 1,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  if (arg_a != arg_b) {\
    instruction_pointer_ = cur_command.args[0].first;\
    return;\
  }\
)
COMMAND(16, "jl", 1, 1,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  if (arg_a < arg_b) {\
    instruction_pointer_ = cur_command.args[0].first;\
    return;\
  }\
)
COMMAND(17, "jle", 1, 1,\
  T arg_b = stack_.extract();\
  T arg_a = stack_.extract();\
\
  if (arg_a <= arg_b) {\
    instruction_pointer_ = cur_command.args[0].first;\
    return;\
  }\
)
COMMAND(18, "ret", 0, 0,\
  instruction_pointer_ = instruction_stack_.extract() + 1;\
  return;\
)
