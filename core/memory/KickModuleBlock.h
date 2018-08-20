#ifndef KICKMODULEBLOCK_6UYSDNAO
#define KICKMODULEBLOCK_6UYSDNAO

#include <memory/MemoryBlock.h>
#include <memory/KickRequestBlock.h>
#include <motion/KickParameters.h>

struct KickModuleBlock : public MemoryBlock {
  NO_SCHEMA(KickModuleBlock);
  KickModuleBlock():
    state_(KickState::NONE)
  {
    header.version = 9;
    header.size = sizeof(KickModuleBlock);
  }

  KickState::State state_;
  Kick::Type kick_type_;
  bool sent_command_;
  bool sent_steady_state_command_;
  float state_start_time_;
  int state_start_frame_;
  float kick_start_time_;
  float align_x_;
  float align_y_;
  float kick2_x_;
  float kick2_y_;
  float cropped_align_x_;
  float cropped_align_y_;
  float cropped_kick2_x_;
  float cropped_kick2_y_;
  
  bool set_kick_odometry_;

  Kick::Leg swing_leg_; //leg has been chosen
  float ball_dist_side_; // wrt swing foot - positive if to left, negative if to right
  float ball_dist_forward_; // not used - wrt stance foot

  float desired_kick_angle_;
  float desired_kick_distance_;
};

#endif /* end of include guard: KICKMODULEBLOCK_6UYSDNAO */
