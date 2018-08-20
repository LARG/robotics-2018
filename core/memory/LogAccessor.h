#pragma once

#include <common/Util.h>
#include <unordered_set>

class LogAccessor {
  public:
    static std::string logPath(std::string directory) { return directory + "/frames.log"; }
    static bool containsLog(std::string directory) {
      return util::fexists(logPath(directory));
    }
    void setEnabledBlocks(std::vector<std::string> blocks) {
      enabled_blocks_ = std::unordered_set<std::string>(blocks.begin(), blocks.end());
    }
    inline bool valid(std::string block) const {
      if(enabled_blocks_.empty()) return true;
      return enabled_blocks_.find(block) != enabled_blocks_.end();
    }
  protected:
    std::unordered_set<std::string> enabled_blocks_;
};
