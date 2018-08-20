#ifndef LOGGER_3AB7OTVI
#define LOGGER_3AB7OTVI

#include <fstream>
#include <memory/LogMetadata.h>
#include <memory/MemoryFrame.h>
#include <memory/MemoryBlock.h>
#include <common/InterfaceInfo.h>
#include <memory/StreamBuffer.h>
#include <memory/LogAccessor.h>
#include <sys/stat.h>

class LogWriter : public LogAccessor {
  public:
    virtual ~LogWriter ();

    inline void writeFrame(const MemoryFrame& frame) { writeMemory(frame); }
    void writeMemory(const MemoryFrame& memory);
    bool isOpen();
    void open(const char *directory, bool appendUniqueId = false);
    std::string directory() const { return directory_; }
    void close();
    void write();
    void setType(int type);
    void clearBuffer();
    inline const StreamBuffer& getBuffer() const { return main_buffer_; }

  protected:
    LogWriter(bool useBuffers, const char* directory, bool appendUniqueId, bool useAllBlocks = false);
    LogMetadata& mdata() { return mdata_; }
    std::string generateDirectoryName(const char *basename);
    void writeMemoryHeader(const MemoryHeader &header, StreamBuffer& buffer);

    std::ofstream log_file_;
    std::string directory_, filename_;
    int type_;
    int frame_id_ = 0;

    bool using_buffers_, use_all_blocks_;
    StreamBuffer main_buffer_;
    LogMetadata mdata_;
};

class StreamLogWriter : public LogWriter {
  public:
    StreamLogWriter() : LogWriter(true, NULL, false) { }
};

class FileLogWriter : public LogWriter {
  public:
    FileLogWriter() : LogWriter(false, NULL, true) { }
};

class LogViewer;
class EditLogWriter : public LogWriter {
  public:
    EditLogWriter(const char* directory) : LogWriter(false, directory, false, true) { }
    EditLogWriter(const std::string& directory) : EditLogWriter(directory.c_str()) { }

    void copyFrame(const LogViewer& viewer, int sframe, int tframe);
};

#endif /* end of include guard: LOGGER_3AB7OTVI */
