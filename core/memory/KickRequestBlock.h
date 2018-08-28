#ifndef KICKREQUESTBLOCK_WDRQVCMO
#define KICKREQUESTBLOCK_WDRQVCMO

#include <memory/MemoryBlock.h>
#include <common/RobotInfo.h>
#include <common/RingBufferWithSum.h>
#include <iostream>
#include <string>

// for display in tool
const std::string kickTypeNames[] = {
  "NO_KICK",
  "STRAIGHT",
  "IKSTRAIGHT",
  "SIDE",
  "ANGLE",
  "DEG30",
  "ABORT"
};

// for display in tool
const std::string legNames[] = {
  "LEFT",
  "RIGHT",
  "SWITCHABLE"
};

class Kick { // class for python access, otherwise namespace
public:
  enum Type {
    NO_KICK,
    STRAIGHT,
    IKSTRAIGHT,
    SIDE,
    ANGLE,
    DEG30,
    ABORT
  };

  enum Leg {
    LEFT,
    RIGHT,
    SWITCHABLE
  };
};

struct KickRequestBlock : public MemoryBlock {
  NO_SCHEMA(KickRequestBlock);
public:
  KickRequestBlock():
  kick_type_(Kick::NO_KICK),
    kick_leg_(Kick::RIGHT),
    desired_angle_(0.0),
    desired_distance_(3000),
    ball_image_center_x_(0),
    ball_image_center_y_(0),
    ball_seen_(false),
    new_head_(true),
    kick_running_(false),
    vision_kick_running_(false),
    allow_correction_walk_(false),
    ball_rel_x_(0),
    ball_rel_y_(0),
    finished_with_step_(false),
    kick_aborted_(false)
  {
    header.version = 10;
    header.size = sizeof(KickRequestBlock);
  }

  void set(Kick::Type kick, Kick::Leg leg, float desired_angle, float desired_distance) {
    kick_type_ = kick;
    kick_leg_ = leg;
    desired_angle_ = desired_angle;
    desired_distance_ = desired_distance;
    //std::cout << "kick_request_->kick_type_: " << kick_type_ << std::endl;
  }

  void setNoKick() {
    set(Kick::NO_KICK,Kick::LEFT,0.0,3000);
  }

  void setFwdKick(Kick::Leg leg=Kick::RIGHT, float desired_distance=1000.0f) {
    set(Kick::STRAIGHT,leg,0.0,desired_distance);
  }

  void setSideKick(Kick::Leg leg, float desired_distance) {
    float angle = DEG_T_RAD * 90;
    if (leg == Kick::SWITCHABLE) { // side kicks are not switchable !
      std::cout << "WARNING: side kicks are not switchable, setting kick request to none" << std::endl;
      setNoKick();
      return;
    }
    if (leg == Kick::LEFT) // not sure of directions here 
      angle *= -1; //we ingore the angle anyway
    set(Kick::SIDE,leg,angle,desired_distance);
  }

  void setAngleKick(Kick::Leg leg, float desired_angle, float desired_distance) {
    set(Kick::ANGLE,leg,desired_angle,desired_distance);
  }

  void setDeg30Kick(Kick::Leg leg, float desired_angle, float desired_distance) {
    set(Kick::DEG30,leg, desired_angle, desired_distance);
  }

  void abortKick(){
    kick_type_ = Kick::ABORT;
  }

  void finishedWithStep() {
    finished_with_step_ = true;
  }

  void initAccelX() {
    accelX_.init();
  }

  void addAccelX(float accel) {
    accelX_.add(accel);
  }

  float getAvgAccelX() {
    return accelX_.getAverage();
  }

  void initAccelY() {
    accelX_.init();
  }

  void addAccelY(float accel) {
    accelY_.add(accel);
  }

  float getAvgAccelY() {
    return accelY_.getAverage();
  }

  Kick::Type kick_type_;
  Kick::Leg kick_leg_;
  float desired_angle_; // in radians
  float desired_distance_; // in mm

  int ball_image_center_x_;
  int ball_image_center_y_;
  bool ball_seen_;
  bool new_head_;

  bool kick_running_;
  bool vision_kick_running_;
  bool allow_correction_walk_;

  float ball_rel_x_;
  float ball_rel_y_;

  bool finished_with_step_;
  RingBufferWithSum<float, 10> accelX_;
  RingBufferWithSum<float, 10> accelY_;

  bool kick_aborted_;
};

#endif /* end of include guard: KICKREQUESTBLOCK_WDRQVCMO */
