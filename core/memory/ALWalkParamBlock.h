#ifndef ALWALKPARAMBLOCK_H
#define ALWALKPARAMBLOCK_H

#include <memory/MemoryBlock.h>

// for sending/display
const unsigned int NUM_AL_WALK_PARAMS = 7;
const std::string walkParamNames[NUM_AL_WALK_PARAMS] = {
  "MaxStepX",
  "MaxStepY",
  "MaxStepTheta",
  "MaxStepFrequency",
  "StepHeight",
  "TorsoWx",
  "TorsoWy",
};

struct ALWalkParamBlock : public MemoryBlock {
  NO_SCHEMA(ALWalkParamBlock);
  ALWalkParamBlock():
    send_params_(false),
    maxStepX(0.04),
    maxStepY(0.14),
    maxStepTheta(0.0349066),
    maxStepFrequency(1.0),
    stepHeight(0.02),
    torsoWx(0.0),
    torsoWy(0.0),
    fwd_odometry_factor_(1.0),
    side_odometry_factor_(1.0),
    turn_cw_odometry_factor_(1.0),
    turn_ccw_odometry_factor_(1.0)
  {
    header.version = 5;
    header.size = sizeof(ALWalkParamBlock);
  }
  
  bool send_params_;
  
  // parameters for aldebaran walk 
  float maxStepX;
  float maxStepY;
  float maxStepTheta;
  float maxStepFrequency;
  float stepHeight;
  float torsoWx;
  float torsoWy;

  // odometry factors
  float fwd_odometry_factor_;
  float side_odometry_factor_;
  float turn_cw_odometry_factor_;
  float turn_ccw_odometry_factor_;

};

#endif 
