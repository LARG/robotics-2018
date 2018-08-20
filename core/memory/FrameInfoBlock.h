#ifndef FRAMEINFO_KJS7E8UX
#define FRAMEINFO_KJS7E8UX

#include <memory/MemoryBlock.h>
#include <iostream>
#include <schema/gen/FrameInfoBlock_generated.h>

enum MemorySource {
  MEMORY_ROBOT,
  MEMORY_SIM
};

DECLARE_INTERNAL_SCHEMA(struct FrameInfoBlock : public MemoryBlock {
  SCHEMA_METHODS(FrameInfoBlock);
public:
  FrameInfoBlock(unsigned int frame_id = 0, double seconds_since_start = 0, MemorySource source = MEMORY_ROBOT):
    frame_id(frame_id),
    start_time(-1),
    seconds_since_start(seconds_since_start),
    source(source)
  {
    header.version = 0;
    header.size = sizeof(FrameInfoBlock);
    log_block = true;
  }
  SCHEMA_FIELD(uint32_t frame_id);
  SCHEMA_FIELD(double start_time);
  SCHEMA_FIELD(double seconds_since_start);
  SCHEMA_FIELD(MemorySource source);
});

#endif /* end of include guard: FRAMEINFO_KJS7E8UX */
