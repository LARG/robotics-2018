#ifndef WALKENGINEPARAMETERS_5GLKWI3S
#define WALKENGINEPARAMETERS_5GLKWI3S

#include <math/Pose2D.h>
#include <math/Vector3.h>

struct WalkEngineParameters {
  WalkEngineParameters();
  // all distances refer to mm unless otherwise specified
  // angles are all in radians
  // times are all in seconds

  // max speeds
  Pose2D max_step_size_;
  // dimensions of walk
  float foot_separation_;
  float walk_height_; // desired dist floor to pendulum.z - tied to offline calculations
  float pendulum_height_; // how high is our pendulum point above the ground

  float step_height_;

  float phase_length_; // seconds for 1 step by 1 foot

  float double_support_frac_;

  int accel_sensor_delay_frames_;
  int pen_sensor_delay_frames_;

  int num_averaged_sensor_zmp_frames_;
  int num_averaged_sensor_pen_frames_;

  bool closed_loop_zmp_;
  bool closed_loop_pen_;
  
  bool interp_zmp_forward_;
  float interp_zmp_side_amount_;

  float zmp_sensor_control_ratio_;
  float pen_sensor_control_ratio_;

  float tilt_roll_factor_;

  float min_step_change_time_; // we can change steps unless they're farther off than this time

  // ref zmp offsets
  Vector2<float> left_foot_zmp_offset_;
  Vector2<float> right_foot_zmp_offset_;

  // both lift and step timings are proportions of the single support phase
  float lift_start_time_;
  float lift_stop_time_;
  // step fwd/side timings
  float step_start_time_;
  float step_stop_time_;
  float step_speed_factor_; // factor for sigmoid

  // hip roll offset trapezoid
  float hip_roll_offset_amount_;
  float hip_roll_offset_rise_frac_;
  float hip_roll_offset_fall_frac_;
  float hip_roll_offset_start_frac_;
  float hip_roll_offset_stop_frac_;

  // tilt offsets to swing foot
  float swing_tilt_amount_;
  float swing_tilt_start_frac_;
  float swing_tilt_stop_frac_;
};

#endif /* end of include guard: WALKENGINEPARAMETERS_5GLKWI3S */
