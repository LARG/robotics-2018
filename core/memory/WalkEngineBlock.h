#ifndef WALKENGINEBLOCK_CTX0HMI2
#define WALKENGINEBLOCK_CTX0HMI2

#include <memory/MemoryBlock.h>
#include <memory/WalkRequestBlock.h>
#include <math/Pose2D.h>
#include <math/Pose3D.h>
#include <common/RingQueue.h>
#include <common/RingQueueWithSum.h>

#define MAX_PREVIEW_FRAMES 100
#define MAX_SENSOR_DELAY 25

struct WalkEngineBlock : public MemoryBlock {
  NO_SCHEMA(WalkEngineBlock);
  WalkEngineBlock() {
    header.version = 19;
    header.size = sizeof(WalkEngineBlock);
    current_stiffness_ = 0.0;
  }

  struct Step {
    Pose2D position_;
    //float time_;
    unsigned int frame_;
    bool is_left_foot_;
    bool is_stand_;

    // initialize values
    Step(){
      //time_ = 0.0;
      frame_ = 0;
      is_left_foot_ = false;
      is_stand_ = false;
    }
  };

  struct WalkState {
    Vector2<float> pen_pos_;
    Vector2<float> pen_vel_;
    Vector2<float> zmp_;
  };

  // what walk are we trying to execute
  Pose2D desired_step_size_;
  Pose2D previous_step_size_;

  // current foot destinations
  float groin_;
  Pose3D swing_foot_;

  Pose3D abs_left_foot_;
  Pose3D abs_right_foot_;

  // steps
  Step step_prev_; // stored for calculating the swing leg
  Step step_current_;
  Step step_next_;
  Step step_after_next_;
  Step step_two_after_next_;

  // frame of reference offsets
  Pose2D global_frame_offset_;
  Pose2D global_to_odometry_frame_offset_;
  Pose2D global_last_torso_;

  // zmp
  RingQueue<Vector2<float>, MAX_PREVIEW_FRAMES> zmp_ref_;

  Vector2<float> sum_zmp_errors_;
  Vector2<float> sensor_zmp_;
  Vector2<float> current_control_;
  Vector3<float> sensor_pen_;

  WalkState current_state_;
  WalkState desired_next_state_;
  WalkState desired_next_without_closed_loop_;

  // for delayed sensor updates
  
  // keep old state around to compare with sensor readings when we get them
  RingQueue<Vector2<float>, MAX_SENSOR_DELAY> delayed_zmp_state_buffer_;
  RingQueue<Vector3<float>, MAX_SENSOR_DELAY> delayed_pen_state_buffer_;

  // keep accel/step around to compare with pen readings when we get them
  RingQueue<Vector2<float>, MAX_SENSOR_DELAY> delayed_accel_sensor_buffer_;
  RingQueue<Step, MAX_SENSOR_DELAY> delayed_step_buffer_;

  Vector2<float> delayed_zmp_state_;
  Vector2<float> delayed_accel_sensor_;
  Vector3<float> delayed_pen_state_;
  Step delayed_stance_step_;

  RingQueueWithSum<Vector2<float>,MAX_SENSOR_DELAY> buffered_sensor_zmp_;
  RingQueueWithSum<Vector3<float>,MAX_SENSOR_DELAY> buffered_sensor_pen_;
  
  // what motion is being executed
  WalkRequestBlock::Motion motion_current_;
  WalkRequestBlock::Motion motion_prev_;
  float time_motion_started_;

  // where we are in the walk
  float phase_;

  unsigned int num_step_frames_;
  unsigned int num_double_support_frames_;

  float current_stiffness_;
};

#endif /* end of include guard: WALKENGINEBLOCK_CTX0HMI2 */
