#ifndef KINEMATICS_MODULE_H
#define KINEMATICS_MODULE_H

#include <Module.h>

#include <common/MassCalibration.h>
#include <common/RobotDimensions.h>

struct SensorBlock;
struct JointBlock;
struct BodyModelBlock;
struct RobotInfoBlock;

class KinematicsModule: public Module {
 public:
  void specifyMemoryDependency();
  void specifyMemoryBlocks();

  void calculatePose();
  void calculatePose(float* joints, float* sensors, float* dimensions, BodyModelBlock* body_model);
  void calcRelCenterOfMassFromFSRs(float* sensors, float* dimensions, BodyModelBlock* body_model_block);

 private:
  // memory blocks
  SensorBlock* sensors_;
  JointBlock* joints_;
  BodyModelBlock* body_model_;
  RobotInfoBlock* robot_info_;

  // Variables and counters for knowing if we are being carried
  int frames_in_air_;
};

#endif /* end of include guard: SENSOR_MODULE */
