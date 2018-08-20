#ifndef PROCESSEDSONARBLOCK_VCLB37XJ
#define PROCESSEDSONARBLOCK_VCLB37XJ

#include <memory/MemoryBlock.h>
#include <schema/gen/ProcessedSonarBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct ProcessedSonarBlock : public MemoryBlock {
  SCHEMA_METHODS(ProcessedSonarBlock);

  ProcessedSonarBlock() {

    header.version = 3;
    header.size = sizeof(ProcessedSonarBlock);

    on_left_ = false;
    on_right_ = false;
    on_center_ = false;
    left_distance_ = 0;
    right_distance_ = 0;
    center_distance_ = 0;

    bump_left_ = false;
    bump_right_ = false;

    sonar_module_update_ = false;
    sonar_module_enabled_ = true;
  }

  SCHEMA_FIELD(bool on_left_);
  SCHEMA_FIELD(bool on_right_);
  SCHEMA_FIELD(bool on_center_);

  SCHEMA_FIELD(float left_distance_);
  SCHEMA_FIELD(float right_distance_);
  SCHEMA_FIELD(float center_distance_);

  SCHEMA_FIELD(bool sonar_module_enabled_);
  SCHEMA_FIELD(bool sonar_module_update_);

  SCHEMA_FIELD(bool bump_left_);
  SCHEMA_FIELD(bool bump_right_);
});

#endif /* end of include guard: PROCESSEDSONARBLOCK_VCLB37XJ */
