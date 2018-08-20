#pragma once

#include <string>
#include <vector>

namespace yuview {
  class FileNavigator {
    public:
      FileNavigator(const std::string& initialPath);
      std::string nextPath(int keyCode);
      static constexpr int ArrowRight = 1113939, ArrowLeft = 1113937, ArrowUp = 1113938, ArrowDown = 1113940;
      bool quit(int keyCode);
    private:
      std::vector<std::string> files_;
      std::string current_;
  };
}
