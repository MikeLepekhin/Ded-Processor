#include <iostream>
#include <getopt.h>

#include "common_classes.h"
#include "assembler.h"
#include "disassembler.h"
#include "executor.h"

void printLine(const std::string& text) {
  std::cout << "# " << text << '\n';
}

void printError(const std::string& text) {
  std::cerr << "!!! " << text << '\n';
}

void help() {
  printLine("Hello, dear user!");
  printLine("this assembler interpreter has next work modes:");
  printLine("--a [asm_file] [binary_file] : assembler");
  printLine("--d [binary_file] [asm_file] : disassembler");
  printLine("--e [binary_file] : execution of binary_file");
  printLine("--i [binary_file] : execution of source_file");
}

void parseOptions(int argc, char** argv, char* mode) {
  const char* const short_opts = "v:h";
  const option long_opts[] = {
    {"verbose", no_argument, nullptr, 'v'},
    {"help", no_argument, nullptr, 'h'},
    {"assembler", no_argument, nullptr, 'a'},
    {"disassembler", no_argument, nullptr, 'd'},
    {"executor", no_argument, nullptr, 'e'},
    {"interpreter", no_argument, nullptr, 'i'},
    {nullptr, no_argument, nullptr, 0}
  };

  int opt = 0;
  while (opt != -1) {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    switch (opt) {
      case 'h':
        help();
        exit(0);
        break;
      case 'a':
        *mode = 'a';
        break;
      case 'd':
        *mode = 'd';
        break;
      case 'e':
        *mode = 'e';
        break;
      case 'i':
        *mode = 'i';
        break;
      case -1:
        return;
      case '?':
      default:
        printError(std::string("!!! unknown option ") + static_cast<char>(opt));
        return;
    }
  }
}

void myAssembler(int argc, char* argv[]) {
  SmartFile asm_file(argc > 2 ? fopen(argv[2], "r") : stdin);
  SmartFile binary_file(argc > 3 ? fopen(argv[3], "wb") : stdout);

  try {
    printLine(std::string("input file: ") + (argc > 2 ? argv[2] : "stdin"));
    printLine(std::string("output file: ") + (argc > 3 ? argv[3] : "stdout"));

    assembly(asm_file.getFile(), binary_file.getFile());
  } catch (ProcessorException& exc) {
    std::cerr << exc;
    exit(1);
  }
}

void myDisassembler(int argc, char* argv[]) {
  SmartFile binary_file(argc > 2 ? fopen(argv[2], "rb") : stdin);
  SmartFile decode_file(argc > 3 ? fopen(argv[3], "w") : stdout);

  try {
    printLine(std::string("input file: ") + (argc > 2 ? argv[2] : "stdin"));
    printLine(std::string("output file: ") + (argc > 3 ? argv[3] : "stdout"));

    disassembly(binary_file.getFile(), decode_file.getFile());
  } catch (ProcessorException& exc) {
    std::cerr << exc;
    exit(1);
  }
}

void myExecutor(int argc, char* argv[]) {
  SmartFile binary_file(argc > 2 ? fopen(argv[2], "rb") : stdin);
  printLine(std::string("input file: ") + (argc > 2 ? argv[2] : "stdin"));

   try {
     execute(binary_file.getFile());
   } catch (ProcessorException& exc) {
     std::cerr << exc;
     exit(1);
   }
}

void myInterpreter(int argc, char* argv[]) {
  try {
    myAssembler(argc, argv);
    argv[2] = argv[3];
    myExecutor(argc, argv);
  } catch (ProcessorException& exc) {
    std::cerr << exc;
  }
}

int main(int argc, char* argv[]) {
  char mode = '0';

  parseOptions(argc, argv, &mode);
  if (argc == 1 && mode == '0') {
    help();
    return 0;
  }
  if (mode != 'a' && mode != 'd' & mode != 'e' && mode != 'i') {
    printError(std::string("Incorrect mode: ") + mode);
    return 0;
  }
  switch (mode) {
    case 'a':
      myAssembler(argc, argv);
      break;
    case 'd':
      myDisassembler(argc, argv);
      break;
    case 'e':
      myExecutor(argc, argv);
      break;
    case 'i':
      myInterpreter(argc, argv);
      break;
  }
  return 0;
}