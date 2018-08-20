#include <memory/TextLogger.h>
#include <chrono>

// Fill in this unordered_set with levels that you want to restrict to
// for example, const std::unordered_set<int> TextLogger::ValidLevels = { 10, 20, 25 };
const std::unordered_set<int> TextLogger::ValidLevels = { };
constexpr bool TextLogger::Enabled;

TextLogger::TextLogger(const std::string& filename, bool appendUniqueId) : frame_info_(nullptr) {
  if(!filename.empty()) {
    open(filename, appendUniqueId);
  }
  clearFilters();
}

TextLogger::~TextLogger() {
  close();
}

void TextLogger::open(const std::string& filename, bool appendUniqueId) {
  close(); // any previously opened textlog
  if(appendUniqueId) {
    std::string path = generateUniqueFileName(filename, "txt");
    std::cout << "Text logging to file " << path << std::endl;
    text_file_.open(path);
  } else {
    std::cout << "Text logging to file " << filename << std::endl;
    text_file_.open(filename);
  }
  enabled_ = true;
}

void TextLogger::close() {
  if(enabled_) {
    enabled_ = false;
    std::cout << "Closing text log file" << std::endl;
    text_file_.close();
  }
}

void TextLogger::setFrameInfo(FrameInfoBlock* fi){
  frame_info_ = fi;
}

bool TextLogger::isLevelValid(int logLevel) const {
  if(ValidLevels.size() == 0) return true;
  return ValidLevels.find(logLevel) != ValidLevels.end();
}

void TextLogger::setFilters(int minLevel, int maxLevel, Type moduleType) {
  filters_.enabled = true;
  filters_.minLevel = minLevel;
  filters_.maxLevel = maxLevel;
  filters_.type = moduleType;
}

void TextLogger::clearFilters() {
  filters_.enabled = false;
}

bool TextLogger::isFiltered(int logLevel, Type moduleType) {
  if(!filters_.enabled) return true;
  if(filters_.type != moduleType) return false;
  if(filters_.minLevel > logLevel) return false;
  if(filters_.maxLevel < logLevel) return false;
  return true;
}

void TextLogger::writeDebug(int loglevel, int frame, TextLogger::Type moduleType, const std::string& s) {
  auto mtype = static_cast<int>(moduleType);
  if(online_mode_){
    auto buffer = tfm::format("loglev %d: frame %d: module %d: %s", loglevel, frame, mtype, s.c_str());
    text_entries_.push_back(buffer);
    return;
  }

  // print to file
  if(enabled_) {
    if(loglevel <= file_log_level_)
      tfm::format(text_file_, "loglev %d: frame %d: module %d: %s\n", loglevel, frame, mtype, s.c_str());
  }

  // print to screen
  if(loglevel <= screen_log_level_) {
    tfm::printf("frame %d: %s\n",frame, s.c_str());
  }
}

void TextLogger::setType(int t){
  type_ = t;
}

std::string TextLogger::generateUniqueFileName(const std::string& prefix, const std::string& ext) {
  std::string robotbase = "/home/nao/logs/";
  std::string base = robotbase;
  if (type_ == CORE_SIM) {
    std::string naohome = getenv("NAO_HOME");
    std::string simbase = naohome + "/logs/";
    base = simbase;
  } else if (type_ == CORE_TOOL) {
    std::string naohome = getenv("NAO_HOME");
    std::string toolbase = naohome + "/logs/";
    base = toolbase;
  }

  std::string logFile = base + prefix + "_%y_%m_%d-%H_%M_%S/frames." + ext;
  using namespace std::chrono;
  auto now = system_clock::to_time_t(system_clock::now());
  auto path = std::put_time(std::localtime(&now), logFile.c_str());
  return tfm::format("%s", path);
}
