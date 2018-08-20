#include "KinematicsModule.h"

#include <kinematics/ForwardKinematics.h>
#include <common/RobotInfo.h>

#include <memory/BodyModelBlock.h>
#include <memory/JointBlock.h>
#include <memory/SensorBlock.h>
#include <memory/RobotInfoBlock.h>

#include <iostream>

void KinematicsModule::specifyMemoryDependency() {
  requiresMemoryBlock("processed_sensors");
  requiresMemoryBlock("processed_joint_angles");
  requiresMemoryBlock("body_model");
  requiresMemoryBlock("robot_info");
}

void KinematicsModule::specifyMemoryBlocks() {
  getMemoryBlock(sensors_,"processed_sensors");
  getMemoryBlock(joints_,"processed_joint_angles");
  getMemoryBlock(body_model_,"body_model");
  getMemoryBlock(robot_info_,"robot_info");

  frames_in_air_ = 0;
}

void KinematicsModule::calculatePose() {
  calculatePose(joints_->values_.data(), sensors_->values_.data(), robot_info_->dimensions_.values_, body_model_);
  ForwardKinematics::calculateCoM(body_model_->abs_parts_.data(),body_model_->center_of_mass_, robot_info_->mass_calibration_);
}

void KinematicsModule::calculatePose(float* joints, float* sensors, float* dimensions, BodyModelBlock* body_model) {

  ForwardKinematics::calculateRelativePose(joints, 0, 0, body_model->rel_parts_.data(), dimensions);

  Pose3D base = ForwardKinematics::calculateVirtualBase(sensors, body_model->rel_parts_.data());
  // Calculate absolute pose from the virtual base
  ForwardKinematics::calculateAbsolutePose(base, body_model->rel_parts_.data(), body_model->abs_parts_.data());
  body_model->is_calculated_ = true;
  

  // See if feet are on ground
  float totalForce = 0;
  for (int i=fsrLFL; i<=fsrRRR; i++) {
    totalForce += sensors[i];
  }
  if (totalForce < 1.0) {
    frames_in_air_++;
    body_model->feet_on_ground_inst_ = false;
  } else {
    frames_in_air_=0;
    body_model->feet_on_ground_inst_ = true;
  }
  if (frames_in_air_ > 75) { // Increased from 25 for Allison (51) since foot sensors zero out easy
    if (body_model->feet_on_ground_) std::cout << "BodyModel: I'm flying !\n";
    body_model->feet_on_ground_=false;
  } else {
    body_model->feet_on_ground_=true;
  }

  calcRelCenterOfMassFromFSRs(sensors, dimensions, body_model);
}

void KinematicsModule::calcRelCenterOfMassFromFSRs(float* sensors, float* dimensions, BodyModelBlock* body_model) {
  body_model->zmpFromFSRs.x = 0;
  body_model->zmpFromFSRs.y = 0;
  Vector3<float> point;
  float totalForce = 0;
  int ind;
  BodyPart::Part foot = BodyPart::left_bottom_foot;
  int sign = 1;
  for (int i = fsrLFL; i <= fsrRRR; i++) {
    ind = (i - fsrLFL) % 4;
    if ((i - fsrLFL) == 4) { // how is this even possible
      //std::cout << " | ";
      foot = BodyPart::right_bottom_foot;
      sign = -1;
    }
    Pose3D &startPos = body_model->rel_parts_[foot];
    Vector3<float> offset;
    switch(i - fsrLFL) {
      case 0:
          offset = Vector3<float>(
            dimensions[RobotDimensions::FSR_LFL_Offset1], 
            dimensions[RobotDimensions::FSR_LFL_Offset2], 
            dimensions[RobotDimensions::FSR_LFL_Offset3]
          );
        break;
      case 1:
          offset = Vector3<float>(
            dimensions[RobotDimensions::FSR_LFR_Offset1], 
            dimensions[RobotDimensions::FSR_LFR_Offset2], 
            dimensions[RobotDimensions::FSR_LFR_Offset3]
          );
        break;
      case 2:
          offset = Vector3<float>(
            dimensions[RobotDimensions::FSR_LRL_Offset1], 
            dimensions[RobotDimensions::FSR_LRL_Offset2], 
            dimensions[RobotDimensions::FSR_LRL_Offset3]
          );
      case 3:
          offset = Vector3<float>(
            dimensions[RobotDimensions::FSR_LRR_Offset1], 
            dimensions[RobotDimensions::FSR_LRR_Offset2], 
            dimensions[RobotDimensions::FSR_LRR_Offset3]
          );
        break;
    }
    offset.y *= sign;

    point = Pose3D(startPos).translate(offset).translation;
    if (sensors[i] > 0.1) {
      body_model->zmpFromFSRs.x += point.x * sensors[i];
      body_model->zmpFromFSRs.y += point.y * sensors[i];
      totalForce += sensors[i];
    }
    //std::cout << sensors->values_[i] << " ";
    //std::cout << point.x << " " << point.y << " | ";
    //std::cout << startPos.translation.y << " ";
  }
  //std::cout << std::endl;
  // normalize for the total force
  body_model->zmpFromFSRs /= totalForce;

  //std::cout << body_model->zmpFromFSRs.x << " " << body_model->zmpFromFSRs.y << std::endl;
}
