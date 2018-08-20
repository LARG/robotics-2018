#ifndef RSWALKPARAMETERS_BENPHJGZ
#define RSWALKPARAMETERS_BENPHJGZ

#include <math/Pose2D.h>

// only a subset of the full parameters, for now
struct RSWalkParameters {
  RSWalkParameters():
    set(false)
  {}

  bool set;
  Pose2D speedMax;
  Pose2D speedMaxChange;
  float speed;

  float max_forward_ = 0.3;
  float max_left_ = 0.2;
  float max_turn_ = 0.87;
  float forward_change_ = 0.15;
  float left_change_= 0.2;
  float turn_change_ = 0.8;

  float com_offset_ = 0.01;
  float walk_hip_height_ = 0.23;
  float base_leg_lift_ = 0.012;
  float base_walk_period_ = 0.23;

  float arm_swing_ = 6.f;
  float pendulum_height_ = 300.0f;
  float forward_extra_foot_height_ = 0.01;
  float left_extra_foot_height_ = 0.02;
  float start_lift_divisor_ = 3.5;
  float max_percent_change_ = 1.0;
};


#endif /* end of include guard: RSWALKPARAMETERS_BENPHJGZ */