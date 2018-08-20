#ifndef SIMIMAGEBLOCK_
#define SIMIMAGEBLOCK_

#include <common/RobotInfo.h>

#include <memory/MemoryBlock.h>

struct SimImageBlock : public MemoryBlock {
  NO_SCHEMA(SimImageBlock);
public:
  SimImageBlock()  {
    header.version = 0;
    header.size = sizeof(SimImageBlock);
  }

  char image_[SIM_IMAGE_SIZE];
};

#endif 
