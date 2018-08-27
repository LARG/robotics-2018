#pragma once
#include <string>

class CommandLineProcessor {
  public:
    static int runLogServer(std::string source, bool loop);
    static int runBehaviorSim();
    static int runLocalizationSim();
};
