#ifndef LOG_H
#define LOG_H

#include <memory/MemoryFrame.h>
#include <memory/ImageBlock.h>
#include <common/RobotInfo.h>
#include <common/ImageBuffer.h>
#include <vector>
#include <unordered_map>
#include <common/Util.h>

class LogWriter;
class LogReader;
class LogMetadata;

class LogViewer {
  public:
    LogViewer(std::string directory, int start = 0, int finish = -1);
    ~LogViewer();
    inline virtual bool cached() const { return false; }
    inline MemoryFrame& operator[] (unsigned int idx) {
      return getFrame(idx);
    }
    inline const MemoryFrame& operator[] (unsigned int idx) const {
      return getFrame(idx);
    }
    virtual MemoryFrame& getFrame(unsigned int idx) const;
    std::vector<ImageParams> getTopParams() const;
    std::vector<ImageParams> getBottomParams() const;
    std::vector<ImageBuffer> getRawTopImages() const;
    std::vector<ImageBuffer> getRawBottomImages() const;
    ImageParams getTopParams(int frame) const;
    ImageParams getBottomParams(int frame) const;
    ImageBuffer getRawTopImage(int frame) const;
    ImageBuffer getRawBottomImage(int frame) const;
    inline std::string name() const { return util::getFileFromPath(directory_); }
    std::size_t size() const { return finish_ - start_ + 1; }
    const std::string& directory() const { return directory_; }

    const LogMetadata& mdata() const { return *mdata_; }
    
    LogReader& reader() { return *reader_; }
    const LogReader& reader() const { return *reader_; }
    
  private:
    std::unique_ptr<LogReader> reader_;
    std::unique_ptr<LogMetadata> mdata_;
    mutable std::unique_ptr<MemoryFrame> mframe_;

  protected:
    std::string directory_;
    int start_, finish_;
};

class CachedLogViewer : public LogViewer { 
  public:
    using LogViewer::LogViewer;
    ~CachedLogViewer() = default;
    inline bool cached() const final { return true; }
    MemoryFrame& getFrame(unsigned int idx) const final;

  private:
    mutable std::unordered_map<int, std::unique_ptr<MemoryFrame>> cache_;
};

using Log = LogViewer;
using CachedLog = CachedLogViewer;

#endif
