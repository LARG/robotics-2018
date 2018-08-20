#include "WalkEngineParameters.h"

#include <math/Geometry.h>

WalkEngineParameters::WalkEngineParameters():
  // max speeds
  max_step_size_(DEG_T_RAD*30,50,40), // Apparently the constructor is Rot,X,Y not X,Y,Rot (MQ 3/17/2011)
  // dimensions of walk
  foot_separation_(2 * 50),
  walk_height_(202), // desired dist floor to com.z - tied to offline calculations
  pendulum_height_(310),
  step_height_(20),
  phase_length_(0.45),
  double_support_frac_(0.1),
  accel_sensor_delay_frames_(2),
  pen_sensor_delay_frames_(6),
  num_averaged_sensor_zmp_frames_(1),
  num_averaged_sensor_pen_frames_(3),
  closed_loop_zmp_(false),
  closed_loop_pen_(false),
  interp_zmp_forward_(true),
  interp_zmp_side_amount_(20.0),
  zmp_sensor_control_ratio_(0.3),
  pen_sensor_control_ratio_(0.3),
  tilt_roll_factor_(0),
  min_step_change_time_(0.3),
  left_foot_zmp_offset_(0,0),
  right_foot_zmp_offset_(0,0),
  // both lift and step timings are proportions of the single support phase
  lift_start_time_(0.0),
  lift_stop_time_(1.0),
  step_start_time_(0.0),
  step_stop_time_(1.0),
  step_speed_factor_(20.0),
  hip_roll_offset_amount_(DEG_T_RAD * 0),
  hip_roll_offset_rise_frac_(0.04 / 0.50),
  hip_roll_offset_fall_frac_(0.04 / 0.50),
  hip_roll_offset_start_frac_(0.0),
  hip_roll_offset_stop_frac_(1.0),
  swing_tilt_amount_(DEG_T_RAD * 5),
  swing_tilt_start_frac_(0.5),
  swing_tilt_stop_frac_(1.0)
{
}
