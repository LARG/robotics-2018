#include "SensorModule.h"

#include <common/RobotInfo.h>

#include <memory/SensorBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/SensorCalibrationBlock.h>

#define DEBUG_SENSOR_CALIBRATION
#define CALIBRATE_ONCE
//#define CALIBRATE_ACCELS

void SensorModule::specifyMemoryDependency() {
  requiresMemoryBlock("raw_sensors");
  requiresMemoryBlock("processed_sensors");
  requiresMemoryBlock("frame_info");
  requiresMemoryBlock("walk_request");
  requiresMemoryBlock("sensor_calibration");
  requiresMemoryBlock("body_model");
}

void SensorModule::specifyMemoryBlocks() {
  getMemoryBlock(raw_sensors_,"raw_sensors");
  getMemoryBlock(sensors_,"processed_sensors");
  getMemoryBlock(frame_info_,"frame_info");
  getMemoryBlock(walk_request_,"walk_request");
  getOrAddMemoryBlock(sensor_calibration_,"sensor_calibration");
  getMemoryBlock(body_model_,"body_model");
}


void SensorModule::initSpecificModule() {
  inertial_filter_.init(frame_info_->source == MEMORY_SIM);
  sensor_calibration_->is_calibrated_ = false;
  last_temperature_check_time_ = -1.0;
}

void SensorModule::processSensors() {
  raw_sensors_->copyPrevValues();
  sensors_->copyPrevValues();
  for (int i=0; i<NUM_SENSORS; i++) {
      sensors_->values_[i] = raw_sensors_->values_[i];
  }
  sensors_->values_[accelX] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56
  sensors_->values_[accelY] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56
  sensors_->values_[accelZ] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56

  for (int i=0; i<NUM_JOINTS; i++) {
    sensors_->joint_temperatures_[i] = raw_sensors_->joint_temperatures_[i];
  }

  for (int i=0; i<NUM_SONAR_VALS; i++) {
    sensors_->sonar_left_[i] = raw_sensors_->sonar_left_[i];
    sensors_->sonar_right_[i] = raw_sensors_->sonar_right_[i]; 
  }

//  // Debugging sonar
//  std::cout << "Left sonar readings: ";
//  for (int i=0; i<NUM_SONAR_VALS; i++) {
//    std::cout << sensors_->sonar_left_[i] << ",";
//  }
//  std::cout << std::endl;
//  std::cout << "Right sonar readings: ";
//  for (int i=0; i<NUM_SONAR_VALS; i++) {
//    std::cout << sensors_->sonar_right_[i] << ",";
//  }
//  std::cout << std::endl;


  filterInertial();
  filterFSR();
  handleCalibration();
  checkTemperatures();
}
  
void SensorModule::filterInertial() {
  if (body_model_->is_calculated_) {
    inertial_filter_.updateIUHeight(body_model_->abs_parts_[BodyPart::torso].translation.z);
  }
  // apply calibration to the inertial sensors
  for (int i = 0; i < 6; i++) {
    int ind = i + gyroX;
    float offset = sensor_calibration_->inertial_offsets_[i];
#ifndef CALIBRATE_ACCELS
    if (i >= 3)
      offset = 0;
#endif
    //sensors_->values_[ind] = raw_sensors_->values_[ind] + offset;
    sensors_->values_[ind] = sensors_->values_[ind] + offset;
  }
  
  // don't filter the body tilt and roll if we're not calibrated
  if (sensor_calibration_->is_calibrated_) {
    // get the filtered tilt and roll
    inertial_filter_.setInertialData(sensors_->values_[accelX],sensors_->values_[accelY],sensors_->values_[accelZ],sensors_->values_[gyroX],sensors_->values_[gyroY]);
    inertial_filter_.processFrame(sensor_calibration_->is_calibrated_);
    //std::cout << "ald " << RAD_T_DEG * sensors_->values_[angleX] << " " << RAD_T_DEG * sensors_->values_[angleY] << " " << 0 << std::endl;
    sensors_->values_[angleX] = inertial_filter_.getRoll();// + sensor_calibration_->roll_offset_;
    sensors_->values_[angleY] = inertial_filter_.getTilt();// + sensor_calibration_->tilt_offset_;
    sensors_->angleXVel = inertial_filter_.getRollVel();
    sensors_->angleYVel = inertial_filter_.getTiltVel();
    inertial_filter_.predictFuture(7,sensors_->futureTilt,sensors_->futureRoll);
    //std::cout << "ut  " << RAD_T_DEG * sensors_->values_[angleX] << " " << RAD_T_DEG * sensors_->values_[angleY] << " " << 0 << std::endl;
    //std::cout << "accel: " << sensors_->values_[accelX] << " " << sensors_->values_[accelY] << " " << sensors_->values_[accelZ] << std::endl;
    //std::cout << "gyro: " << sensors_->values_[gyroX] << " " << sensors_->values_[gyroY] << std::endl;
  }
  //if (sensor_calibration_->is_calibrated_)
  //std::cout << "tr " << RAD_T_DEG * sensors_->values_[angleX] << " " << RAD_T_DEG * sensors_->values_[angleY] << " " << 0 << std::endl;
  //std::cout << "ald " << RAD_T_DEG * sensors_->futureRoll << " " << RAD_T_DEG * sensors_->futureTilt << " " << 0 << std::endl;
    //std::cout << "raw: " << raw_sensors_->values_[accelZ] << " filt: " << sensors_->values_[accelZ] << std::endl;
    //std::cout << "calibration: " << RAD_T_DEG * sensor_calibration_->tilt_offset_ << " " << RAD_T_DEG * sensor_calibration_->roll_offset_ << std::endl;
  //}
}

float SensorModule::sumFSRs(int inds[], int num_fsrs) {
  float sum = 0;
  for (int i = 0; i < num_fsrs; i++)
    sum += sensors_->values_[inds[i]];
  return sum;
}

void SensorModule::filterFSR() {
  int left[4] = {fsrLFL,fsrLFR,fsrLRL,fsrLRR};
  int right[4] = {fsrRFL,fsrRFR,fsrRRL,fsrRRR};
  int left_left[2] = {fsrLFL,fsrLRL};
  int left_right[2] = {fsrLFR,fsrLRR};
  int left_front[2] = {fsrLFL,fsrLFR};
  int left_rear[2] = {fsrLRL,fsrLRR};
  int right_left[2] = {fsrRFL,fsrRRL};
  int right_right[2] = {fsrRFR,fsrRRR};
  int right_front[2] = {fsrRFL,fsrRFR};
  int right_rear[2] = {fsrRRL,fsrRRR};

  sensors_->fsr_feet_ = sumFSRs(left,4) - sumFSRs(right,4) + sensor_calibration_->fsr_feet_offset_;
  sensors_->fsr_left_side_ = sumFSRs(left_left,2) - sumFSRs(left_right,2) + sensor_calibration_->fsr_left_side_offset_;
  sensors_->fsr_left_front_ = sumFSRs(left_front,2) - sumFSRs(left_rear,2) + sensor_calibration_->fsr_left_front_offset_;
  sensors_->fsr_right_side_ = sumFSRs(right_left,2) - sumFSRs(right_right,2) + sensor_calibration_->fsr_right_side_offset_;
  sensors_->fsr_right_front_ = sumFSRs(right_front,2) - sumFSRs(right_rear,2) + sensor_calibration_->fsr_right_front_offset_;
}

float SensorModule::getOffset(float arr[NUM_CALIBRATION_READINGS]) {
  float sum = 0;
  for (int i = 0; i < NUM_CALIBRATION_READINGS; i++)
    sum += arr[i];
  return sum / NUM_CALIBRATION_READINGS;
}
 
void SensorModule::handleCalibration() {
#ifdef CALIBRATE_ONCE
  if (sensor_calibration_->is_calibrated_)
    return;
#endif
  if (sensor_calibration_->is_calibrated_) {
    // recalibrate carefully

    // only calibrate when we're standing
    if (walk_request_->motion_ != WalkRequestBlock::STAND) {
      stand_start_time_ = -1;
      return;
    }

    // check that the feet are on the ground
    if (!body_model_->feet_on_ground_inst_) {
      // don't calibrate, and throw away previous data
      stand_start_time_ = -1;
      return;
    }
    
    // track the stand time
    if (stand_start_time_ < 0) {
      stand_start_time_ = frame_info_->seconds_since_start;
      calibration_ind_ = 0;
      return;
    }

    // don't calibrate until we've stood safely for a bit
    if (frame_info_->seconds_since_start - stand_start_time_ < 1.5) {
      calibration_ind_ = 0;
      return;
    }
  }

  if (calibration_ind_ == 0) {
    for (int i = 0; i < 6; i++)
      calibration_inertial_sum_[i] = 0;
#ifdef DEBUG_SENSOR_CALIBRATION
    std::cout << "STARTING CALIBRATION" << std::endl;
#endif
  }
  
  // inertial
  for (int i = 0; i < 6; i++) {
    calibration_inertial_sum_[i] += raw_sensors_->values_[i + gyroX];
  }
    
  //// Check errors between kinematics and sensor calculation of tilt/roll on last frame
  //float kin_tilt = (body_model_->left_foot_body_tilt_roll_.tilt_ + body_model_->right_foot_body_tilt_roll_.tilt_)/2.0;
  //float kin_roll = (body_model_->left_foot_body_tilt_roll_.roll_ + body_model_->right_foot_body_tilt_roll_.roll_)/2.0;
  //float tilt_error = kin_tilt - (body_model_->sensors_tilt_roll_.tilt_- sensor_calibration_->tilt_offset_);
  //float roll_error = kin_roll - (body_model_->sensors_tilt_roll_.roll_ - sensor_calibration_->roll_offset_);
  //calibration_tilts_[calibration_ind_] = tilt_error;
  //calibration_rolls_[calibration_ind_] = roll_error;

  //calibration_gyro_x_[calibration_ind_] = sensors_->
      
  // Check errors on fsrs
  //calibration_fsr_feet_[calibration_ind_] = -(sensors_->fsr_feet_ - sensor_calibration_->fsr_feet_offset_);
  //calibration_fsr_left_side_[calibration_ind_] = -(sensors_->fsr_left_side_ - sensor_calibration_->fsr_left_side_offset_);
  //calibration_fsr_left_front_[calibration_ind_] = -(sensors_->fsr_left_front_ - sensor_calibration_->fsr_left_front_offset_);
  //calibration_fsr_right_side_[calibration_ind_] = -(sensors_->fsr_right_side_ - sensor_calibration_->fsr_right_side_offset_);
  //calibration_fsr_right_front_[calibration_ind_] = -(sensors_->fsr_right_front_ - sensor_calibration_->fsr_right_front_offset_);

  calibration_ind_++;

  if (calibration_ind_ >= NUM_CALIBRATION_READINGS) {
    setCalibration();
  }
}

void SensorModule::setCalibration() {
  // inertial
#ifdef DEBUG_SENSOR_CALIBRATION
  std::cout << "New calibration:" << std::endl;
#endif
  for (int i = 0; i < 6; i++) {
    int ind = i + gyroX;
    float baseVal = 0.0;
    if (ind == accelZ)
      baseVal = 9.8;
    //float offset = getOffset(calibration_inertial_[i]);
    float offset = calibration_inertial_sum_[i] / calibration_ind_;
    sensor_calibration_->inertial_offsets_[i] = baseVal - offset;
#ifdef DEBUG_SENSOR_CALIBRATION
    std::cout << "  " << getSensorString((Sensor)ind) << " = " << sensor_calibration_->inertial_offsets_[i] << "(" << offset << ")" << std::endl;
#endif
  }
  //sensor_calibration_->fsr_feet_offset_ = getOffset(calibration_fsr_feet_);
  //sensor_calibration_->fsr_left_side_offset_ = getOffset(calibration_fsr_left_side_);
  //sensor_calibration_->fsr_left_front_offset_ = getOffset(calibration_fsr_left_front_);
  //sensor_calibration_->fsr_right_side_offset_ = getOffset(calibration_fsr_right_side_);
  //sensor_calibration_->fsr_right_front_offset_ = getOffset(calibration_fsr_right_front_);
  
  // finished calibration
  calibration_ind_ = 0;
  sensor_calibration_->is_calibrated_ = true;
}

void SensorModule::checkTemperatures() {
  if (frame_info_->seconds_since_start - last_temperature_check_time_ < 5.0)
    return;
  last_temperature_check_time_ = frame_info_->seconds_since_start;
  float eps = 0.01;
  float hotTemp = 75.0 - eps;
  float warmTemp = 70.0 - eps;
  bool first = true;
  for (int i=0; i<NUM_JOINTS; i++) {
    if (sensors_->joint_temperatures_[i] >= hotTemp) {
      if (first) {
        first = false;
        std::cout << "Hot joints: " << std::endl;
      }
      std::cout << "  " << getJointName((Joint)i) << ": " << sensors_->joint_temperatures_[i] << std::endl;
    }
  }
  first = true;
  for (int i=0; i<NUM_JOINTS; i++) {
    if ((sensors_->joint_temperatures_[i] >= warmTemp) && (sensors_->joint_temperatures_[i] < hotTemp)) {
      if (first) {
        first = false;
        std::cout << "Warm joints: " << std::endl;
      }
      std::cout << "  " << getJointName((Joint)i) << ": " << sensors_->joint_temperatures_[i] << std::endl;
    }
  }
}
