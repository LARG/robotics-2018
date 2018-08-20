#include "MemoryBlock.h"
#include <iostream>

MemoryBlock::MemoryBlock():
  log_block(false),
  owner(MemoryOwner::UNKNOWN)
{
}

MemoryBlock& MemoryBlock::operator=(const MemoryBlock &that) {
  header = that.header;
  // DON'T COPY log_block or owner
  //std::cout << "operator=" << std::endl;
  return *this;
}

bool MemoryBlock::checkOwner(const std::string &name, MemoryOwner::Owner expect_owner, bool no_exit) const {
  if (owner != expect_owner) {
    // the tool is master of all it surveys
    if (expect_owner == MemoryOwner::TOOL_MEM)
      return true;

    // if it's owned by the interface, and we're the motion thread, it's okay, it should be locked properly
    if (owner == MemoryOwner::INTERFACE && expect_owner == MemoryOwner::MOTION)
      return true;

    // if it's owned by image_capture and we're the vision thread, it should also be locked properly
    if (owner == MemoryOwner::IMAGE_CAPTURE && expect_owner == MemoryOwner::VISION)
      return true;

    if (owner == MemoryOwner::SHARED){
      //std::cout << "accessing shared memory block" << std::endl;
      return true;
    }
    
    if (no_exit){
      //std::cout << "checkOwner: no exit" << std::endl;
      return false;
    }
    std::cerr << "BAD OWNER FOR: " << name << " expected: " <<  expect_owner << " " << " got: " << owner << std::endl;
    exit(1);
  }
  return true;
}

bool MemoryBlock::validateHeader(const StreamBuffer& thbuffer) {
  MemoryBlockHeader theader;
  memcpy((unsigned char*)&theader, thbuffer.buffer, sizeof(MemoryBlockHeader));
  return validateHeader(theader);
}

bool MemoryBlock::validateHeader(const MemoryBlockHeader& theader) {
  if(theader.version != header.version) {
    fprintf(stderr, "ERROR reading header: Log Version: %i, Current Version: %i\n", theader.version, header.version);
    return false;
  }
  if(theader.size != header.size) {
    fprintf(stderr, "ERROR reading header: Log Size: %i, Current Size: %i\n", theader.size, header.size);
    return false;
  }
  return true;
}
