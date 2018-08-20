#pragma once

namespace yuview {
  namespace Util {
    std::vector<uint8_t> fread(std::string source);
    std::vector<std::string> listImages(const std::string& directory);
    std::string getDirectory(std::string path);
    void mkdirp(const std::string& directory);
  };
}
