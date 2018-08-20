#include <vector>
#include <stdint.h>
#include <string>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

namespace yuview {
  using namespace std;

  namespace Util {
    vector<uint8_t> fread(string path) {
      basic_ifstream<char> stream(path, ios::in | ios::binary);    
      auto eos = istreambuf_iterator<char>();
      auto cbuffer = vector<char>(istreambuf_iterator<char>(stream), eos);
      vector<uint8_t> buffer(cbuffer.size());
      copy(cbuffer.begin(), cbuffer.end(), reinterpret_cast<char*>(buffer.data()));
      return buffer;
    }
  
    vector<string> listFiles(const string& directory, const vector<string>& extensions) {
      vector<string> files;
      DIR *dir;
      struct dirent *entry;
      if(dir = opendir(directory.c_str())) {
        while(entry = readdir(dir)) {
          string name = entry->d_name;
          int index = name.find_last_of('.') + 1;
          if(index < name.length()) {
            string extension = name.substr(index, name.length() - index);
            for(const auto& ext : extensions) {
              if(extension == ext) {
                files.push_back(directory + "/" + name);
                break;
              }
            }
          }
        }
      }
      std::sort(files.begin(), files.end());
      return files;
    }

    vector<string> listImages(const string& directory) {
      return listFiles(directory, { "yuv" });
    }

    string getFilename(string path) {
      string filename = path;
      size_t pos = path.find_last_of("/");
      if(pos != string::npos)
        filename.assign(path.begin() + pos + 1, path.end());
      return filename;
    }

    string getDirectory(string path) {
      string directory = "";
      size_t pos = path.find_last_of("/");
      if(pos != string::npos)
        directory.assign(path.begin(), path.begin() + pos);
      return directory;
    }

    void mkdirp(const std::string& directory) {
      char tmp[256];
      char *p = NULL;
      size_t len;

      snprintf(tmp, sizeof(tmp),"%s",directory.c_str());
      len = strlen(tmp);
      if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
      for(p = tmp + 1; *p; p++)
        if(*p == '/') {
          *p = 0;
          mkdir(tmp, S_IRWXU);
          *p = '/';
        }
      mkdir(tmp, S_IRWXU);
    }
  }
}
