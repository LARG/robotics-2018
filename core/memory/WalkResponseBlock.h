#ifndef WALKRESPONSEBLOCK_RE8SDRLN
#define WALKRESPONSEBLOCK_RE8SDRLN

#include <memory/MemoryBlock.h>

struct WalkResponseBlock : public MemoryBlock {
  NO_SCHEMA(WalkResponseBlock);
public:
  WalkResponseBlock():
    finished_standing_(false)
  {
    header.version = 1;
    header.size = sizeof(WalkResponseBlock);
  }
  
  bool received_;
  bool finished_standing_;
};

#endif /* end of include guard: WALKRESPONSEBLOCK_RE8SDRLN */
