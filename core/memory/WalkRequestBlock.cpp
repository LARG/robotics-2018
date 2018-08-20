#include <memory/WalkRequestBlock.h>

WalkRequestBlock::WalkRequestBlock():
  new_command_(false),
  motion_(NONE),
  speed_(),
  percentage_speed_(true),
  pedantic_walk_(false),
  walk_to_target_(false),
  target_walk_is_active_(false),
  rotate_around_target_(false),
  rotate_heading_(0),
  perform_kick_(false),
  kick_heading_(0),
  kick_distance_(3000),
  kick_with_left_(false),
  step_into_kick_(false),
  set_kick_step_params_(false),
  tilt_fallen_counter_(0),
  roll_fallen_counter_(0),
  getup_from_keeper_dive_(false),
  odometry_fwd_offset_(0),
  odometry_side_offset_(0),
  odometry_turn_offset_(0),
  keep_arms_out_(false),
  dive_type_(Dive::NONE),
  slow_stand_(false),
  stand_in_place_(false),
  walk_type_(INVALID_WALK),
  walk_control_status_(WALK_CONTROL_OFF)
{
  header.version = 18;
  header.size = sizeof(WalkRequestBlock);
}
void WalkRequestBlock::set(Motion m, Pose2D speed, bool percentage_speed, bool pedantic) {
  new_command_ = true;
  motion_ = m;
  speed_ = speed;
  percentage_speed_ = percentage_speed;
  pedantic_walk_ = pedantic;
  walk_to_target_ = false;
  rotate_around_target_ = false;
  perform_kick_ = false;
  kick_heading_ = 0;
  kick_distance_ = 3000;
  kick_with_left_ = false;
  step_into_kick_ = false;
  rotate_distance_ = 0;
  rotate_heading_ = 0;
}

void WalkRequestBlock::noWalk() {
  set(NONE,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::stand() {
  finished_standing_ = false;
  set(STAND,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::standStraight() {
  finished_standing_ = false;
  set(STAND_STRAIGHT,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::standPenalty() {
  finished_standing_ = false;
  set(STAND_PENALTY,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::wait() {
  set(WAIT,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::setStep(bool isLeft, float x, float y, float rotation) {
  Motion walk = STEP_RIGHT;
  if (isLeft) walk = STEP_LEFT;
  set(walk,Pose2D(rotation,x,y),false,false);
}

void WalkRequestBlock::setWalk(float x, float y, float rotation) {
  set(WALK,Pose2D(rotation,x,y),true,false);
  walk_control_status_ = WALK_CONTROL_OFF;
  is_rotate_ = false;
}

void WalkRequestBlock::setPedanticWalk(float x, float y, float rotation) {
  set(WALK,Pose2D(rotation,x,y),true,true);
}

void WalkRequestBlock::setFalling() {
  set(FALLING,Pose2D(0,0,0),true,false);
}

void WalkRequestBlock::setKick(float distance, float heading, bool with_left, bool step_into_kick){
  new_command_ = true;
  motion_ = WALK;
  perform_kick_ = true;
  kick_heading_ = heading;
  kick_distance_ = distance;
  kick_with_left_ = with_left;
  step_into_kick_ = step_into_kick;
}

void WalkRequestBlock::setLineUp(float relx, float rely, float rot, bool with_left)
{
  new_command_ = true;
  motion_ = LINE_UP;
  kick_heading_ = 0.0;//heading;
  kick_with_left_ = with_left;
  target_point_.translation.x = relx;
  target_point_.translation.y = rely;
  target_point_.rotation = rot;//heading;
  is_rotate_ = false;
  
}
    
void WalkRequestBlock::setLineUpParameters(float forward_gap, float left_gap, float forward_over_threshold, float forward_under_threshold, float left_threshold, float max_forward, float max_left, float done_ct, float turn_speed) {

    kick_forward_gap_ = forward_gap;
    kick_left_gap_ = left_gap;
    kick_over_forward_threshold_ = forward_over_threshold;
    kick_under_forward_threshold_ = forward_under_threshold;
    kick_left_threshold_ = left_threshold;
    kick_max_forward_ = max_forward;
    kick_max_left_ = max_left;
    kick_done_ct_ = done_ct; 
    kick_turn_speed_ = turn_speed;
}

void WalkRequestBlock::setWalkKick(float relx, float rely, float rot, bool with_left, float heading)
{
  setLineUp(relx, rely, rot, with_left);
  motion_ = WALK;
  perform_kick_ = true;
  kick_heading_ = heading;
  is_rotate_ = false;
  
}
void WalkRequestBlock::setOdometryOffsets(float fwd, float side, float turn) {
  odometry_fwd_offset_ = fwd;
  odometry_side_offset_ = side;
  odometry_turn_offset_ = turn;
}

void WalkRequestBlock::setWalkTarget(float relx, float rely, float relang, bool pedantic) {
  new_command_ = true;
  motion_ = WALK;
  walk_to_target_ = true;
  rotate_around_target_ = false;
  target_point_.translation.x = relx;
  target_point_.translation.y = rely;
  target_point_.rotation = relang;
  pedantic_walk_ = pedantic;
}

void WalkRequestBlock::setKickStepParams(int type, const Pose2D &preStep, const Pose2D &step, float refX) {
  set_kick_step_params_ = true;
  step_kick_type_ = type;
  pre_kick_step_ = preStep;
  kick_step_ = step;
  kick_step_ref_x_ = refX;
}

void WalkRequestBlock::setWalkType(WalkType type) {
  walk_type_ = type;
}

