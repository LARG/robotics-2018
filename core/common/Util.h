#pragma once

#include <string>
#include <initializer_list>
#include <common/tinyformat.h>
#include <cassert>
#include <vector>

namespace util {

  template<typename... Args>
  [[deprecated("Use util::format instead.")]]
  inline std::string ssprintf(const std::string& format, Args&&... args) {
    return tfm::format(format.c_str(), std::forward<Args>(args)...);
  }
  template<typename... Args>
  [[deprecated("Use util::format instead.")]]
  inline std::string ssprintf(const char* format, Args&&... args) {
    return tfm::format(format, std::forward<Args>(args)...);
  }
  template<typename... Args>
  inline std::string format(const std::string& format, Args&&... args) {
    return tfm::format(format.c_str(), std::forward<Args>(args)...);
  }
  template<typename... Args>
  inline std::string format(const char* format, Args&&... args) {
    return tfm::format(format, std::forward<Args>(args)...);
  }
  bool sreplace(std::string& str, const std::string& from, const std::string& to);
  bool sreplace(std::string& str, const std::initializer_list<std::string>& from, const std::string& to);
  bool startswith(const std::string& s, const std::string& start);
  bool endswith(const std::string& s, const std::string& ending);
  std::string ltrim(std::string s);
  std::string rtrim(std::string s);
  std::string trim(const std::string& s);
  bool fexists(std::string path);
  void mkdir_recursive(std::string directory);
  void rmrf(std::string directory);
  std::string env(std::string name);
  std::string getFileFromPath(std::string path);
  std::string getDirectoryFromPath(std::string path);
  void copy(std::string source, std::string target);
  std::vector<std::string> split(const std::string& s, char delim);

  enum ConfigPath {
    Data,
    Models,
    ColorTables,
    FieldConfigs,
    SimViewConfigs,
    GyroConfigs,
    RobotConfig,
    Modules
  };

  std::string cfgpath(ConfigPath config);
}
