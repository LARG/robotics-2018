#pragma once

#include <memory/MemoryBlock.h>
#include <common/States.h>
#include <common/Roles.h>
#include <math/Pose2D.h>
#include <schema/gen/RobotStateBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(class RobotStateBlock : public MemoryBlock {
public:
  SCHEMA_METHODS(RobotStateBlock);
  RobotStateBlock();
  SCHEMA_FIELD(int WO_SELF);
  SCHEMA_FIELD(int global_index_); // Normally WO_SELF, except when we're simulating opponents

  SCHEMA_FIELD(int team_);
  SCHEMA_FIELD(bool team_changed_);

  SCHEMA_FIELD(int robot_id_);  // Which robot serial number are we, -1 = unknown or sim

  SCHEMA_FIELD(int role_);

  SCHEMA_FIELD(bool ignore_comms_);
  SCHEMA_FIELD(double clock_offset_);
  SCHEMA_FIELD(Pose2D manual_pose_);
  SCHEMA_FIELD(float manual_height_);

  SCHEMA_FIELD(int head_version_);
  SCHEMA_FIELD(int body_version_);
  SCHEMA_FIELD(std::array<int8_t,16> body_id_);

  std::string bodyId() { return std::string(reinterpret_cast<const char*>(body_id_.data())).substr(0, 15); }
});
