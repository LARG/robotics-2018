#pragma once

#include <string>
#include <vector>

struct Arguments {
  bool python_debug;
};

class ArgumentParser {
  public:
    static Arguments Parse(int argc, char **argv);
};
