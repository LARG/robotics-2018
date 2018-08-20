#ifndef KICKPARAMBLOCK_FD54EE88
#define KICKPARAMBLOCK_FD54EE88

#include <memory/MemoryBlock.h>
#include <motion/KickParameters.h>

struct KickParamBlock : public MemoryBlock {
  NO_SCHEMA(KickParamBlock);
  KickParamBlock():
    send_params_(false),
    params_(),
    params_super_()
  {
    header.version = 5;
    header.size = sizeof(KickParamBlock);
  }
  
  bool send_params_;
  KickParameters params_;
  KickParameters params_super_;
  KickParameters params_omni_;
  KickParameters params_wide_;
  KickParameters params_stable_;
};

#endif /* end of include guard: KICKPARAMBLOCK_FD54EE88 */
