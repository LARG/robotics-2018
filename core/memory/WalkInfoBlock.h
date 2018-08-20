#ifndef WALKINFOBLOCK_SXELZJMF
#define WALKINFOBLOCK_SXELZJMF

#include <memory/MemoryBlock.h>
#include <math/Pose2D.h>

struct WalkInfoBlock : public MemoryBlock {
  NO_SCHEMA(WalkInfoBlock);
  WalkInfoBlock() {
    header.version = 5;
    header.size = sizeof(WalkInfoBlock);
    walk_is_active_ = false;
  }

  Pose2D robot_odometry_frame_;
  Pose2D robot_last_position_;
  Pose2D robot_position_;
  Pose2D robot_next_position_;
  Pose2D robot_relative_next_position_;
  Pose2D robot_velocity_;
  Pose2D robot_velocity_frac_;

  bool walk_is_active_;

  float instability_;
  bool instable_;
  float stabilizer_on_threshold_;
  float stabilizer_off_threshold_;

  bool is_stance_left_;
  float frac_of_step_completed_;
  float time_remaining_in_step_;

  bool finished_with_target_;
};

#endif /* end of include guard: WALKINFOBLOCK_SXELZJMF */
