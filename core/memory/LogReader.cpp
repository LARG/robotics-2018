#include "LogReader.h"
#include <iostream>
#include <memory/ImageBlock.h>
#include <memory/RobotVisionBlock.h>
#include <memory/MemoryBlockOperations.h>
#include <common/CoreException.h>

#define MAX_EXPECTED_MODULES_PER_MEMORY 40

LogReader::LogReader(const char *directory):
  using_buffers_(false) {
  directory_ = directory;
  filename_ = logPath(directory_);
  log_file_.open(filename_.c_str(),std::ios::binary);
  if(!good()) {
    throw CoreException("Failed to open '%s'", filename_.c_str());
  }
  if(!mdata_.loadFromFile(directory_ + "/metadata.yaml")) {
    throw CoreException("Failed to load metadata for log '%s'", directory);
  }
}

LogReader::LogReader(const std::string& directory) : LogReader(directory.c_str()) { }

LogReader::LogReader(const StreamBuffer& buffer) :
  using_buffers_(true), main_buffer_(buffer) {
}

LogReader::~LogReader() {
  close();
}

bool LogReader::readMemoryHeader(const StreamBuffer& buffer, MemoryHeader& header) const {
  std::vector<StreamBuffer> blockNames;
  StreamBuffer::separate(buffer, blockNames);
  if (blockNames.size() > MAX_EXPECTED_MODULES_PER_MEMORY) {
    std::cout << "LogReader::readMemoryHeader: BAD NUMBER OF BLOCKS: " << blockNames.size() << std::endl;
    return false;
  }
  for(unsigned int i = 0; i < blockNames.size(); i++) {
    std::string name = (const char*)blockNames[i].buffer;
    header.block_names.push_back(name);
  }
  StreamBuffer::clear(blockNames);
  return true;
}

bool LogReader::readBuffer(StreamBuffer& buffer, int frame) const {
  uint64_t p = mdata_.offsets[frame];
  log_file_.seekg(p);
  buffer.read(log_file_);
  return true;
}

MemoryFrame* LogReader::readFrame(int frame) const {
  MemoryFrame* memory = new MemoryFrame(false,MemoryOwner::TOOL_MEM, 0, 1);
  MemoryHeader header;
  uint64_t position = mdata_.offsets[frame];
  log_file_.seekg(position);
  main_buffer_.read(log_file_);
  if(!readMemory(*memory, frame)) {
    printf("Error reading frame %i\n", frame);
  }
  return memory;
}

bool LogReader::readMemory(MemoryFrame &memory, int frame) const {
  // get the header and see how many blocks we need to read
  auto buffers = main_buffer_.separate();

  MemoryHeader header;
  bool res;
  res = readMemoryHeader(buffers[0], header);
  if (!res) {
    StreamBuffer::clear(buffers);
    return false;
  }
  if (header.block_names.size()<1) {
    printf("Error: header has %lu blocks\n", header.block_names.size());
    return false;
  }
  for (unsigned int i = 1; i < buffers.size(); i++) {
    std::string &id = header.block_names[i - 1];
    if(!valid(id)) continue;
    auto block = memory.getBlockPtrByName(id);
    if(!MEMORY_BLOCK_TEMPLATE_FUNCTION_CALL(id, block->is_serializable, false)) continue;
    if (block == NULL) {
      bool res = memory.addBlockByName(id);
      if (!res) {
        std::cout << "Adding block " << id << " failed, just skipping" << std::endl << std::flush;
        continue;
      }
      else {
        block = memory.getBlockPtrByName(id);
        assert(block != NULL); // really shouldn't happen
      }
    }
    block->buffer_logging_ = using_buffers_;
    block->log_block = true; // if we're reading a log we should be able to re-log it
    block->directory_ = directory_.c_str();
    block->frame_id_ = frame;

    serialization::data_pointer loc = buffers[i].buffer;
    bool success = MEMORY_BLOCK_TEMPLATE_FUNCTION_CALL(id, block->deserialize, false, loc);
    block->directory_ = nullptr;
    if(!success)
      fprintf(stderr, "Error deserializing %s\n", id.c_str());
  }
  StreamBuffer::clear(buffers);
  return true;
}

void LogReader::close() {
  if (!using_buffers_)
    log_file_.close();
}

bool LogReader::good() const {
  if (using_buffers_) {
    return true;
  } else
    return log_file_.good();
}
