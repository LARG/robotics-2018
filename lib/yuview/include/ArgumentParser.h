#pragma once

#include <string>
#include <iostream>
namespace yuview {
  class ArgumentParser {
    public:
      enum Action {
        View,
        Convert
      };
      struct Arguments {
        Action action;
        int height, width, size;
        std::string source, target;
        bool verbose, raw;
      };

      static Arguments Parse(int argc, char **argv);
  };

  std::istream& operator>>(std::istream& in, ArgumentParser::Action& action);
}
