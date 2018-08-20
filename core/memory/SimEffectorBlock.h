#ifndef SIMEFFECTORBLOCK_H_
#define SIMEFFECTORBLOCK_H_

#include <common/RobotInfo.h>

#include <memory/MemoryBlock.h>

struct SimEffector {
public:
  SimEffector() {
    scale_ = 1.0;

    current_error_= 0;
    previous_error_= 0;
    cumulative_error_=0;

    current_angle_ = 0;
    target_angle_ = 0;
    target_time_ = 0;
    immediate_target_angle_ = 0;

    k1_ = 0.15;
    k2_ = 0.0;
    k3_ = 0.01;

  }
        
  void setImmediateTargetAngle(const double &tangle){
    immediate_target_angle_ = tangle;
  }

  void updateErrors() {
    previous_error_ = current_error_;
    current_error_ = immediate_target_angle_ - current_angle_;
    cumulative_error_ += current_error_;
  }

  void updateSensorAngle(const double &cangle) {
    current_angle_ = cangle;
    target_time_ -= 20.0;
  }

  void updateCommand(const double &tangle, const double &time, bool change){
    if (change){
      target_angle_ = tangle + current_angle_;
      target_time_ = 20;
    } else {
      target_angle_ = tangle;
      target_time_ = time;
    }
  }



  // Constants for PID control
  double k1_, k2_, k3_;
        
  // Error terms
  double current_error_;// corr. k1
  double cumulative_error_;// corr. k2
  double previous_error_;// corr. k3
  
  double current_angle_;
  double target_angle_;
  double target_time_;
  double immediate_target_angle_;

  float scale_;
};

struct SimEffectorBlock : public MemoryBlock {
  NO_SCHEMA(SimEffectorBlock);
public:
  SimEffectorBlock()  {
    header.version = 1;
    header.size = sizeof(SimEffectorBlock);
  }  

  SimEffector effector_[NUM_JOINTS];
};

#endif 
