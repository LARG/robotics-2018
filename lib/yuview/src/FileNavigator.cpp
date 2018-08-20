#include <FileNavigator.h>
#include <Util.h>
#include <assert.h>
#include <algorithm>
#include <algorithm>
#include <unistd.h>
#include <boost/filesystem.hpp>

using namespace std;

namespace yuview {

  FileNavigator::FileNavigator(const string& initialPath) {
    namespace fs = boost::filesystem;
    auto rpath = fs::path(initialPath);
    auto apath = fs::complete(rpath);
    string directory = Util::getDirectory(apath.string());
    files_ = Util::listImages(directory);
    assert(files_.size() > 0);
    current_ = apath.string();
  }

  std::string FileNavigator::nextPath(int keyCode) {
    assert(files_.size() > 0);
    auto p = std::equal_range(files_.begin(), files_.end(), current_);
    auto it = p.first;
    switch(keyCode) {
      case ArrowRight:
      case ArrowDown:
        if(p.second == files_.end())
          return current_;
        it++; break;
      case ArrowLeft:
      case ArrowUp:
        if(it == files_.begin())
          return current_;
        it--; break;
      default: break;
    }
    current_ = *it;
    return current_;
  }

  bool FileNavigator::quit(int keyCode) {
    switch(keyCode) {
      case ArrowRight:
      case ArrowDown:
      case ArrowLeft:
      case ArrowUp:
        return false;
      default:
        return true;
    }
  }
}
