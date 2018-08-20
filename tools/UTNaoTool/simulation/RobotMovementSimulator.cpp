#include <tool/simulation/RobotMovementSimulator.h>
#include <memory/WalkRequestBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/BehaviorBlock.h>
#include <memory/WalkInfoBlock.h>
#include <memory/OdometryBlock.h>
#include <math/Geometry.h>

#define getObject(obj, idx) auto& obj = world_object_->objects_[idx]
#define SECONDS_PER_FRAME (1.0/30.0)
#define ACCEL_FRAMES 30
#define ROTATE_PERIOD 9
#define ROTATE_EXP 2.5
#define CROP(x,MIN,MAX) x = std::max(std::min(((float)x),((float)(MAX))),((float)(MIN)))

RobotMovementSimulator::RobotMovementSimulator(int player) : player_(player) {
  maxVel_ = Pose2D(130.0 * DEG_T_RAD, 240.0f, 120.0f);
  rotateTimer_ = 0;
  rotatePhase_ = false;
}

void RobotMovementSimulator::setCaches(WorldObjectBlock* gtObjects, MemoryCache bcache) {
  gtObjects_ = gtObjects;
  bcache_ = bcache;
}

Pose2D RobotMovementSimulator::getVelocityRequest() {
  Pose2D reqVel;
  if(bcache_.walk_request->walk_to_target_) {
    auto& self = bcache_.world_object->objects_[player_];
    ctarget_ = bcache_.behavior->absTargetPt;
    enableTarget_ = true;
  }
  auto& robot = bcache_.world_object->objects_[player_];
  if(enableTarget_) {
    auto relTarget = ctarget_.globalToRelative(robot.loc, robot.orientation);
    reqVel.x = CROP(relTarget.x, -maxVel_.x, maxVel_.x);
    reqVel.y = CROP(relTarget.y, -maxVel_.y, maxVel_.y);
    reqVel.t = 0;
  } else if (bcache_.walk_request->percentage_speed_) {
    reqVel.x = maxVel_.x * bcache_.walk_request->speed_.x;
    reqVel.y = maxVel_.y * bcache_.walk_request->speed_.y;
    reqVel.t = maxVel_.t * bcache_.walk_request->speed_.t;
  }
  return reqVel;
}

void RobotMovementSimulator::step() {
  resetWalkInfo();
  auto& bSelf = gtObjects_->objects_[player_];
  if(bcache_.walk_request->motion_ != WalkRequestBlock::WALK && bcache_.walk_request->motion_ != WalkRequestBlock::WAIT && !bcache_.walk_request->walk_to_target_){
    enableTarget_ = false;
  } else if(enableTarget_ && ctarget_.getDistanceTo(bSelf.loc)) {
    enableTarget_ = false;
  }
  
  auto& gtSelf = gtObjects_->objects_[player_];
  if(bcache_.walk_request->motion_ == WalkRequestBlock::WALK || enableTarget_) {
    auto reqVel = getVelocityRequest();
    Pose2D dVel(reqVel.t - vel_.t, reqVel.x - vel_.x, reqVel.y - vel_.y);
    dVel = dVel / ACCEL_FRAMES;

    vel_ = Pose2D(vel_.t + dVel.t, vel_.x + dVel.x, vel_.y + dVel.y);
    auto velFrac = Pose2D(vel_.t / maxVel_.t, vel_.x / maxVel_.x, vel_.y / maxVel_.y);
    bcache_.walk_info->robot_velocity_ = vel_;
    bcache_.walk_info->robot_velocity_frac_ = velFrac;

    float factor = 0.2f;
    Pose2D movement = vel_ * SECONDS_PER_FRAME;
    movement.t *= pow(((double)rotateTimer_)/(ROTATE_PERIOD/2),ROTATE_EXP);
    bcache_.odometry->displacement = movement;

    movement.x += movement.x * rand_.sampleU(-factor,factor);
    movement.y += movement.y * rand_.sampleU(-factor,factor);
    movement.t += movement.t * rand_.sampleU(-factor,factor);
    Point2D gtrans(movement.x, movement.y);
    gtrans = gtrans.rotate(gtSelf.orientation);
    gtSelf.loc += gtrans;
    gtSelf.orientation += movement.t;

    bcache_.walk_info->walk_is_active_ = true;
    bcache_.odometry->standing = false;
    if(fabs(reqVel.t) < EPSILON) {
      rotateTimer_ = 0;
      rotatePhase_ = false;
    } else {
      if(rotateTimer_ == 0) rotatePhase_ = true;
      if(rotateTimer_ == ROTATE_PERIOD - 1) rotatePhase_ = false;
      if(rotatePhase_)
        rotateTimer_ = (rotateTimer_ + 1);
      else
        rotateTimer_ = (rotateTimer_ - 1);
    }
    //printf("Req: %2.f,%2.f @ %2.f, Vel: %2.f,%2.f @ %2.f, D Vel: %2.2f,%2.2f @ %2.2f, movement: %2.f,%2.f @ %2.2f, global: %2.f, %2.f, self: %2.f,%2.f @ %2.f\n",
      //reqVel.x, reqVel.y, reqVel.t * RAD_T_DEG,
      //vel_.x, vel_.y, vel_.t * RAD_T_DEG,
      //dVel.x, dVel.y, dVel.t * RAD_T_DEG,
      //movement.x, movement.y, movement.t * RAD_T_DEG,
      //gtrans.x, gtrans.y,
      //gtSelf.loc.x, gtSelf.loc.y, gtSelf.orientation * RAD_T_DEG
    //);
  }
}

void RobotMovementSimulator::resetWalkInfo() {
  bcache_.walk_info->robot_velocity_ = Pose2D(0,0,0);
  bcache_.walk_info->robot_velocity_frac_ = Pose2D(0,0,0);
  bcache_.walk_info->walk_is_active_ = false;
  bcache_.odometry->displacement = Pose2D(0,0,0);
}
