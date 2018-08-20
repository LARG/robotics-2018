#ifndef WALKPARAMBLOCK_REVJHZ5T
#define WALKPARAMBLOCK_REVJHZ5T

#include <memory/MemoryBlock.h>
#include <motion/BHWalkParameters.h>
#include <motion/RSWalkParameters.h>

struct WalkParamBlock : public MemoryBlock {
  NO_SCHEMA(WalkParamBlock);
  WalkParamBlock():
    send_params_(false),
    use_sprint_params_(false)
  {
    header.version = 11;
    header.size = sizeof(WalkParamBlock);
  }

  bool send_params_;
  bool use_sprint_params_;
  bool can_sprint_;
  RSWalkParameters main_params_;
  RSWalkParameters sprint_params_;

  // TODO: Put in an actual computation for this
  // This is needed for some tool drawing functionality
  float walkHeight() { return 400; }
};

#endif /* end of include guard: WALKPARAMBLOCK_REVJHZ5T */
