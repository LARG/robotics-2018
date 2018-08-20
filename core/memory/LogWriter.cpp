#include <memory/LogWriter.h>
#include <iostream>
#include <common/File.h>
#include <memory/ImageBlock.h>
#include <memory/RobotVisionBlock.h>
#include <memory/MemoryBlockOperations.h>
#include <memory/LogViewer.h>
#include <memory/LogReader.h>
#include <common/Util.h>

LogWriter::LogWriter(bool useBuffers, const char* directory, bool appendUniqueId, bool useAllBlocks):
  using_buffers_(useBuffers), use_all_blocks_(useAllBlocks)
{
  if (directory) {
    open(directory, appendUniqueId);
  }
}

LogWriter::~LogWriter() {
  if(using_buffers_) {
    if(main_buffer_.buffer)
      delete [] main_buffer_.buffer;
  }
  close();
}

bool LogWriter::isOpen() {
  return log_file_.is_open();
}

void LogWriter::clearBuffer() {
  main_buffer_.reset();
}

void LogWriter::writeMemoryHeader(const MemoryHeader &header, StreamBuffer& buffer) {
  unsigned int num_blocks = header.block_names.size();
  std::vector<StreamBuffer> buffers;
  for (unsigned int i = 0; i < num_blocks; i++) {
    if(!valid(header.block_names[i])) continue;
    StreamBuffer bbuffer;
    bbuffer.read(header.block_names[i].c_str(), header.block_names[i].size() + 1);
    buffers.push_back(bbuffer);
  }
  StreamBuffer::combine(buffers, buffer);
}

void LogWriter::writeMemory(const MemoryFrame &memory) {
  if (using_buffers_ || log_file_.is_open()) {
    mdata_.frames++;
    mdata_.offsets.push_back(log_file_.tellp());
    MemoryHeader header;
    // first get a list of the blocks we're loading
    memory.getBlockNames(header.block_names, !use_all_blocks_);
    // write the header
    StreamBuffer hbuffer;
    writeMemoryHeader(header, hbuffer);
    // write each block
    std::vector<serialization::serializer> serializers;
    for (unsigned int i = 0; i < header.block_names.size(); i++) {
      std::string& id = header.block_names[i];
      if(!valid(id)) continue;
      auto block = memory.getBlockPtrByName(id);
      serializers.emplace_back();
      if(!MEMORY_BLOCK_TEMPLATE_FUNCTION_CALL(id, block->is_serializable, false)) continue;
      auto& serializer = serializers.back() = serialization::create_serializer();
      block->buffer_logging_ = using_buffers_;
      block->frame_id_ = frame_id_;
      block->directory_ = directory_.c_str();
      
      auto loc = MEMORY_BLOCK_TEMPLATE_FUNCTION_CALL(id, block->serialize, NULL, serializer);
      serializer->Finish(loc);
      block->directory_ = nullptr;
    }
    std::vector<StreamBuffer> buffers;
    buffers.push_back(hbuffer);
    for(auto& serializer : serializers) {
      if(serializer == nullptr) buffers.emplace_back();
      else buffers.emplace_back(serializer->GetBufferPointer(), serializer->GetSize());
    }
    StreamBuffer::combine(buffers, main_buffer_);
    if(!using_buffers_) {
      write();
      // Write out metadata every 100 frames just in case of a crash
      if(mdata_.frames % 100 == 0) mdata_.saveToFile(directory_ + "/metadata.yaml");
    }
    // No need to clear because the memory is shared with serializers 
    // which are automatically destructed properly
    // StreamBuffer::clear(buffers); 
    hbuffer.clear();
    frame_id_++;
  }
}

void LogWriter::open(const char *directory, bool appendUniqueId) {
  close(); // any previously opened logs
  mdata_.frames = 0;
  mdata_.offsets.clear();
  if (appendUniqueId) {
    directory_ = generateDirectoryName(directory);
  } else {
    directory_ = directory;
  }
  util::mkdir_recursive(directory_);
  filename_ = logPath(directory_);
  printf("Logging to file: %s\n", filename_.c_str());
  log_file_.open(filename_.c_str(), std::ios::binary);
}

void LogWriter::close() {
  if (log_file_.is_open()) {
    std::cout << "Closing log file" << std::endl;
    log_file_.close();
    mdata_.saveToFile(directory_ + "/metadata.yaml");
  }
  frame_id_ = 0;
}

void LogWriter::setType(int t){
  type_ = t;
}

std::string LogWriter::generateDirectoryName(const char* basename) {
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

  std::string directory = base + std::string(basename) + generateTimestamp();
  return directory;
}

void LogWriter::write() {
  main_buffer_.write(log_file_);
}

void EditLogWriter::copyFrame(const LogViewer& viewer, int sframe, int tframe) {
  // Update metadata
  mdata().frames++;
  mdata().offsets.push_back(log_file_.tellp());

  // Read the source data into the buffer
  StreamBuffer buffer;
  viewer.reader().readBuffer(buffer, sframe);
  
  // Write the buffer into the output log file
  buffer.write(log_file_);

  // Copy over auxiliary files (just images)
  auto tsource = util::format("%s/top_%04i.yuv", viewer.directory(), sframe);
  auto ttarget = util::format("%s/top_%04i.yuv", directory_, tframe);
  if(util::fexists(tsource))
    util::copy(tsource, ttarget);
  
  auto bsource = util::format("%s/bottom_%04i.yuv", viewer.directory(), sframe);
  auto btarget = util::format("%s/bottom_%04i.yuv", directory_, tframe);
  if(util::fexists(bsource))
    util::copy(bsource, btarget);
}
