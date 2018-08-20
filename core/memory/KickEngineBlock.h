#ifndef KICKENGINEBLOCK_
#define KICKENGINEBLOCK_

#include <memory/MemoryBlock.h>
#include "KickRequestBlock.h"

// for tool debug display
const std::string kickStateNames[] = {
  "STAND",
  "WAIT",
  "STANDTALL",
  "SHIFT",
  "ONELEG",
  "KICK",
  "FOOTMIDDLE",
  "FOOTDOWN",
  "FINISHSTANDTALL",
  "FINISHSTAND",
  "ABORT",
  "NONE"
};

struct KickEngineBlock : public MemoryBlock {
  NO_SCHEMA(KickEngineBlock);
public:
  enum Kick_State {
    STAND,
    WAIT,
    STANDTALL,
    SHIFT,
    ONELEG,
    KICK,
    FOOTMIDDLE,
    FOOTDOWN,
    FINISHSTANDTALL,
    FINISHSTAND,
    ABORT,
    NONE
  };
  enum Kick_Spec {
    INSIDE,
    OUTSIDE,
    NORMAL
  };

  KickEngineBlock():
    state_(NONE),
    leg_(Kick::LEFT),
    type_(Kick::NO_KICK)
  {
    header.version = 1;
    header.size = sizeof(KickEngineBlock);
  }

  Kick_State state_; // what state of kick?
  Kick::Leg leg_; //what leg are we kicking with?
  Kick::Type type_;
  Kick_Spec kickSpec_; // middle, inside, or outside?
  float start_time_;
  unsigned int ball_image_center_x_;
  unsigned int ball_image_center_y_;
  bool ball_seen_;
  bool first_in_state_;
  float ball_Dist_From_Foot_Side_; // negative is on the inside, positive on the outside - measured from the middle of the foot
  float ball_Dist_From_Foot_Front_; // measured from front middle of foot

  Pose3D current_ankle_; // current ankle x,y,x for IK kicks - updated in each visit to stand, one_leg, kick, and middle
  Pose3D target_ankle_; // the target ankle x,y,z for IK kicks - updated in the first visit to one_leg, kick, and middle
  Pose3D shift_ankle_; // the ankle position when shifted
  Pose3D stand_ankle_; // ankle x,y,x in the stand position for IK kicks - this is the position relative ball distances are calculated at

  float extra_back_one_leg_;
  float foot_up_distance_;

  float desired_kick_distance_; // in mm
  float desired_kick_angle_; // in degrees

  float actual_straight_kick_distance_;
  float actual_side_kick_distance_;
  float actual_Deg30_kick_distance_;

  float stand_joint_time_;
  float shift_joint_time_;
  float one_leg_joint_time_;
  float kick_joint_time_;
  float foot_middle_joint_time_;
  float foot_down_joint_time_;
  float finish_stand_joint_time_;
  float stand_time_;
  float stand_tall_time_;
  float shift_time_;
  float one_leg_time_;
  float kick_time_;
  float foot_middle_time_;
  float foot_down_time_;
  float finish_stand_tall_time_;
  float finish_stand_time_;

};

#endif 
