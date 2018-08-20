#ifndef LOGREADER_ZN55JIC8
#define LOGREADER_ZN55JIC8

#include <fstream>
#include <cstring>
#include <vector>
#include <memory/LogMetadata.h>
#include <memory/StreamBuffer.h>
#include <memory/MemoryFrame.h>
#include <memory/MemoryBlock.h>
#include <memory/LogAccessor.h>

class LogReader : public LogAccessor {
  public:
    LogReader (const char *directory);
    LogReader (const std::string& directory);
    LogReader (const StreamBuffer& buffer);
    ~LogReader ();
    
    MemoryFrame* readFrame(int frame) const;
    bool readMemory(MemoryFrame &memory, int frame) const;
    const LogMetadata& mdata() const { return mdata_; }
    const std::string& directory() const { return directory_; }
    bool readBuffer(StreamBuffer& buffer, int frame) const;

  private:
    bool readMemoryHeader(const StreamBuffer& buffer, MemoryHeader &header) const;
    bool readBlock(const MemoryBlockHeader &header,MemoryBlock &module) const;
    void readAndIgnoreBlock(const MemoryBlockHeader &header) const;
    void read() const;
    bool good() const;
    
    void close();

    bool using_buffers_;
    std::string directory_;
    std::string filename_;
    LogMetadata mdata_;

    mutable std::ifstream log_file_;
    mutable StreamBuffer main_buffer_;
};

#endif /* end of include guard: LOGREADER_ZN55JIC8 */
