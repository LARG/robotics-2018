#include <common/Util.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <system_error>
#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include <vector>

using namespace std;

namespace util {

  /*
  // TODO: remove if ssprintf isn't broken
  string ssprintf(const char* format, ...) {
    const unsigned int bufsize = 4096;
    thread_local char buffer[bufsize];
    va_list args;
    va_start(args, format);
    vsnprintf (buffer, bufsize, format, args);
    va_end (args);
    return string(buffer);
  }
  */

  bool sreplace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
      return false;
    str.replace(start_pos, from.length(), to);
    return true;
  }

  bool sreplace(std::string& str, const std::initializer_list<std::string>& from, const std::string& to) {
    bool rval = true;
    for(const auto& s : from)
      if(!sreplace(str, s, to))
        rval = false;
    return rval;
  }
  
  bool startswith(const std::string& s, const std::string& start) {
    if (s.length() >= start.length())
      return 0 == s.compare(0, start.length(), start);
    return false;
  }

  bool endswith(const std::string& s, const std::string& ending) {
    if (s.length() >= ending.length())
      return 0 == s.compare(s.length() - ending.length(), ending.length(), ending);
    return false;
  }

  // trim from start (in place)
  std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end (in place)
  std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  // trim from both ends (in place)
  std::string trim(const std::string &s) {
    return ltrim(rtrim(s));
  }

  bool fexists(std::string path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
  }

  void mkdir_recursive(std::string directory) {
    auto s = const_cast<char*>(directory.data());
    auto len = directory.length();
    if(s[len - 1] == '/')
      s[len - 1] = 0;
    for(auto p = s + 1; *p; p++)
      if(*p == '/') {
        *p = 0;
        mkdir(s, S_IRWXU);
        *p = '/';
      }
    mkdir(s, S_IRWXU);
  } 

  void rmrf(std::string directory) {
#ifdef TOOL
    boost::filesystem::remove_all(directory);
#else
    fprintf(stderr, "ERROR: util::rmrf is only enabled for tool builds!\n");
    exit(EXIT_FAILURE);
#endif
  }

  string env(string name) {
    char* value = getenv(name.c_str());
    if(!value) {
      throw std::system_error(
        std::make_error_code(std::errc::argument_out_of_domain),
        "The requested environment variable is not set."
      );
    }
    return string(value);
  }
  
  string getFileFromPath(string path) {
    string filename = path;
    size_t pos = path.find_last_of("/");
    if(pos != string::npos)
      filename.assign(path.begin() + pos + 1, path.end());
    return filename;
  }

  string getDirectoryFromPath(string path) {
    string directory = "";
    size_t pos = path.find_last_of("/");
    if(pos != string::npos)
      directory.assign(path.begin(), path.begin() + pos);
    return directory;
  }

  void copy(string ipath, string opath) {
    std::ifstream source(ipath, std::ios::binary);
    if(!source.good())
      throw std::runtime_error(::util::format("Invalid source selected for copy."));
    std::ofstream destination(opath, std::ios::binary);
    if(!destination.good())
      throw std::runtime_error(::util::format("Invalid destination selected for copy."));
    destination << source.rdbuf();
  }
  
  std::vector<std::string> split(const std::string& s, char delim) {
    using namespace std;
    vector<string> elems;
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim))
      elems.push_back(item);
    return elems;
  }

  std::string cfgpath(ConfigPath cfgp) {
#ifdef TOOL
    std::string base = util::env("NAO_HOME") + "/data";
#else
    std::string base = "/home/nao/data";
#endif
    switch(cfgp) {
      case ConfigPath::Modules:
        return ::util::format("%s/modules", base);
      case ConfigPath::ColorTables:
        return ::util::format("%s/current", base);
      case ConfigPath::Models:
        return ::util::format("%s/models", base);
      case ConfigPath::Data:
        return ::util::format("%s", base);
      case ConfigPath::FieldConfigs:
        return ::util::format("%s/simulation/field", base);
      case ConfigPath::SimViewConfigs:
        return ::util::format("%s/simulation/view", base);
      case ConfigPath::GyroConfigs:
        return ::util::format("%s/gyros", base);
      case ConfigPath::RobotConfig:
        return ::util::format("%s/config.yaml", base);
    }
    return "";
  }
}

