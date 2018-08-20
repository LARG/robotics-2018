#ifndef WALKREQUESTBLOCK_RE8SDRLN
#define WALKREQUESTBLOCK_RE8SDRLN

#include <math/Pose2D.h>
#include <memory/MemoryBlock.h>
#include <common/InterfaceInfo.h>
#include <common/Kicks.h>

#include <memory/BehaviorBlock.h>
#include <schema/gen/WalkRequestBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct WalkRequestBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(WalkRequestBlock);
    ENUM(Motion,
      WALK,
      STAND,
      FALLING,
      GETUP,
      FALL_PREVENTION,
      NONE,
      STEP_LEFT,
      STEP_RIGHT,
      WAIT,
      STAND_STRAIGHT,
      LINE_UP,
      STAND_PENALTY,
      NUM_OF_MOTIONS
      );


    WalkRequestBlock();

    void set(Motion m, Pose2D speed, bool percentage_speed, bool pedantic);
    void noWalk();
    void stand();
    void standStraight();
    void standPenalty();
    void wait();
    void setStep(bool isLeft, float x, float y, float rotation);
    void setWalk(float x, float y, float rotation);
    void setPedanticWalk(float x, float y, float rotation);
    void setFalling();
    void setKick(float distance, float heading, bool with_left, bool step_into_kick);
    void setLineUp(float relx, float rely, float rot, bool with_left);
    void setLineUpParameters(float forward_gap, float left_gap, float forward_over_threshold, float forward_under_threshold, float left_threshold, float max_forward, float max_left, float done_ct, float turn_speed); 
    void setWalkKick(float relx, float rely, float rot, bool with_left, float heading=0.0f);
    void setOdometryOffsets(float fwd, float side, float turn);
    void setWalkTarget(float relx, float rely, float relang, bool pedantic = false);
    void setKickStepParams(int type, const Pose2D &preStep, const Pose2D &step, float refX);
    void setWalkType(WalkType type);
    bool isStandingStraight() const { return motion_ == STAND_STRAIGHT; }

    SCHEMA_FIELD(bool new_command_);
    SCHEMA_FIELD(bool start_command_);
    SCHEMA_FIELD(Motion motion_); // what type of motion, walk/stand/kick
    SCHEMA_FIELD(Pose2D speed_); // the speed of the walk
    SCHEMA_FIELD(bool percentage_speed_); // true if speed is percentage rather than absolute vel
    SCHEMA_FIELD(bool pedantic_walk_); // true disables the step size stabilization.  "Set it when precision is indispensable"

    SCHEMA_FIELD(bool is_penalised_);

    // target point walk 
    SCHEMA_FIELD(Pose2D target_point_);
    SCHEMA_FIELD(bool walk_to_target_);
    SCHEMA_FIELD(bool target_walk_is_active_);
    SCHEMA_FIELD(bool finished_standing_);

    SCHEMA_FIELD(bool rotate_around_target_);
    SCHEMA_FIELD(float rotate_distance_);
    SCHEMA_FIELD(float rotate_heading_);

    // for kicking from walk
    SCHEMA_FIELD(bool perform_kick_);
    SCHEMA_FIELD(float kick_heading_);
    SCHEMA_FIELD(float kick_distance_);
    SCHEMA_FIELD(bool kick_with_left_);
    SCHEMA_FIELD(bool step_into_kick_);

    SCHEMA_FIELD(bool set_kick_step_params_);
    SCHEMA_FIELD(int step_kick_type_);
    SCHEMA_FIELD(Pose2D pre_kick_step_);
    SCHEMA_FIELD(Pose2D kick_step_);
    SCHEMA_FIELD(float kick_step_ref_x_);

    SCHEMA_FIELD(int tilt_fallen_counter_);
    SCHEMA_FIELD(int roll_fallen_counter_);
    SCHEMA_FIELD(bool getup_from_keeper_dive_);

    // walk odometry offsets
    SCHEMA_FIELD(float odometry_fwd_offset_);
    SCHEMA_FIELD(float odometry_side_offset_);
    SCHEMA_FIELD(float odometry_turn_offset_);

    // goalie
    SCHEMA_FIELD(bool keep_arms_out_);          // arms
    SCHEMA_FIELD(Dive::diveTypes dive_type_);   // dives

    SCHEMA_FIELD(bool slow_stand_); // true if we need a slow stand

    SCHEMA_FIELD(bool walk_decides_finished_with_target_);
    SCHEMA_FIELD(float finished_with_target_max_x_error_);
    SCHEMA_FIELD(float finished_with_target_min_y_error_);
    SCHEMA_FIELD(float finished_with_target_max_y_error_);

    SCHEMA_FIELD(bool stand_in_place_);

    SCHEMA_FIELD(WalkType walk_type_);

    SCHEMA_FIELD(WalkControl walk_control_status_);

    SCHEMA_FIELD(float kick_forward_gap_);
    SCHEMA_FIELD(float kick_left_gap_);
    SCHEMA_FIELD(float kick_over_forward_threshold_);
    SCHEMA_FIELD(float kick_under_forward_threshold_);
    SCHEMA_FIELD(float kick_left_threshold_);
    SCHEMA_FIELD(float kick_max_forward_);
    SCHEMA_FIELD(float kick_max_left_);
    SCHEMA_FIELD(float kick_done_ct_);
    SCHEMA_FIELD(float kick_turn_speed_);
    
    SCHEMA_FIELD(bool is_rotate_);
});

#endif /* end of include guard: WALKREQUESTBLOCK_RE8SDRLN */
