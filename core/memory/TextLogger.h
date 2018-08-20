#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <common/Enum.h>
#include <memory/FrameInfoBlock.h>
#include <common/InterfaceInfo.h>
#include <common/tinyformat.h>
#include <common/Stream.h>

// module types for text log
ENUM_CLASS(LoggingModule,
  Vision,
  Behavior,
  Localization,
  Opp,
  Kinematics,
  Sensors,
  Audio,
  Communication
);

// If debug logging hasn't been enabled through the build script,
// turn it off by default for robot builds and leave it on for
// tool builds (for running core).
#ifndef ALLOW_DEBUG_LOG
#ifdef TOOL
#define ALLOW_DEBUG_LOG true
#else
#define ALLOW_DEBUG_LOG false
#endif
#endif

#define CREATE_MODULE_LOGGER(Module) \
template<typename... Ts> \
inline void logFrom##Module(int logLevel, const char* format, Ts&&... ts) { \
  if(ALLOW_DEBUG_LOG) \
    log(logLevel, Type::Module, format, std::forward<Ts>(ts)...); \
}

class TextLogger {
  public:
    typedef LoggingModule Type;
    static const std::unordered_set<int> ValidLevels;
    static constexpr bool Enabled = ALLOW_DEBUG_LOG;

    TextLogger (const std::string& filename = "", bool appendUniqueId = false);
    virtual ~TextLogger ();

    std::vector<std::string>& textEntries() { return text_entries_; }
    const std::vector<std::string>& textEntries() const { return text_entries_; }
    bool onlineMode() const { return online_mode_; }
    bool& onlineMode() { return online_mode_; }
    void setFrameInfo(FrameInfoBlock* fi);
    void setType(int t);

    void setFilters(int minLevel, int maxLevel, Type moduleType);
    void clearFilters();
    bool isFiltered(int logLevel, Type moduleType);

    void open(const std::string& filename, bool appendUniqueId = false);
    void close();

    template<typename... Ts>
    inline void log(int logLevel, Type moduleType, const char* format, Ts&&... ts) {
      if(!isLevelValid(logLevel)) return;
      if(!isFiltered(logLevel, moduleType)) return;
      if(frame_info_ == nullptr) return;
      auto s = tfm::format(format, std::forward<Ts>(ts)...);
      writeDebug(logLevel, frame_info_->frame_id, moduleType, s);
    }
    CREATE_MODULE_LOGGER(Vision);
    CREATE_MODULE_LOGGER(Localization);
    CREATE_MODULE_LOGGER(Opp);
    CREATE_MODULE_LOGGER(Audio);
    CREATE_MODULE_LOGGER(Behavior);
    CREATE_MODULE_LOGGER(Communication);

  private:
    bool isLevelValid(int logLevel) const;
    void writeDebug(int loglevel, int frame, Type moduleType, const std::string& s);
    std::string generateUniqueFileName(const std::string& prefix, const std::string& ext);

    bool enabled_ = false, online_mode_ = false;
    FrameInfoBlock* frame_info_ = nullptr;
    int screen_log_level_ = 5;
    int file_log_level_ = 100;

    int type_;
    std::ofstream text_file_;
    std::vector<std::string> text_entries_;
    std::unordered_set<int> filtered_levels_;
    struct { Type type; int minLevel; int maxLevel; bool enabled; } filters_;
};
