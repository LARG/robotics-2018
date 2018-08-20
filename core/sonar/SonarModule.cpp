#include "SonarModule.h"
#include <iomanip>

#include <common/RobotInfo.h>

#include <memory/FrameInfoBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SensorBlock.h>
#include <memory/ProcessedSonarBlock.h>
#include <memory/RobotStateBlock.h>

using namespace Sonar;

void SonarModule::specifyMemoryDependency() {
  requiresMemoryBlock("frame_info");
  requiresMemoryBlock("processed_joint_commands");
  requiresMemoryBlock("raw_sensors");
  requiresMemoryBlock("processed_sonar");
  requiresMemoryBlock("robot_state");
}

void SonarModule::specifyMemoryBlocks() {
  getMemoryBlock(frame_info_,"frame_info");
  getMemoryBlock(commands_,"processed_joint_commands");
  getMemoryBlock(sensors_,"raw_sensors");
  getMemoryBlock(processed_sonar_,"processed_sonar");
  getMemoryBlock(robot_state_,"robot_state"); 
}

void SonarModule::initSpecificModule() {

  // Setup names for easy read
  command_map_[Sonar::LEFT_TO_LEFT] = "LL";
  command_map_[Sonar::LEFT_TO_RIGHT] = "LR";
  command_map_[Sonar::RIGHT_TO_LEFT] = "RL";
  command_map_[Sonar::RIGHT_TO_RIGHT] = "RR";
  command_map_[Sonar::LEFT_TO_LEFT + Sonar::RECEIVE_BOTH] = "LL+B";
  command_map_[Sonar::LEFT_TO_RIGHT + Sonar::RECEIVE_BOTH] = "LR+B";
  command_map_[Sonar::RIGHT_TO_LEFT + Sonar::RECEIVE_BOTH] = "RL+B";
  command_map_[Sonar::RIGHT_TO_RIGHT + Sonar::RECEIVE_BOTH] = "RR+B";
  command_map_[Sonar::TRANSMIT_BOTH + Sonar::RECEIVE_BOTH] = "BOTH";
  command_map_[Sonar::RESET] = "RESET";

  if (robot_state_->body_version_ >= 50) {
    min_distance_ = MIN_DISTANCE;
    max_distance_ = MAX_DISTANCE_V5;
    lr_distance_diff_ = LR_DISTANCE_DIFF_V5;
    //std::cout << "V5 robot, set distances for sonar to " << min_distance_ << ", " << max_distance_ << ", " << lr_distance_diff_ << std::endl;
  } else {
    min_distance_ = MIN_DISTANCE;
    max_distance_ = MAX_DISTANCE_V4;
    lr_distance_diff_ = LR_DISTANCE_DIFF_V4;
    //std::cout << "V4 robot, set distances for sonar to " << min_distance_ << ", " << max_distance_ << ", " << lr_distance_diff_ << std::endl;
  }

  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::LEFT_TO_LEFT);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::LEFT_TO_RIGHT);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::RIGHT_TO_LEFT);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::RIGHT_TO_RIGHT);
  // modes_.push_back(Sonar::LEFT_TO_LEFT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::LEFT_TO_RIGHT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::RIGHT_TO_LEFT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RESET);
  // modes_.push_back(Sonar::RIGHT_TO_RIGHT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RESET);
  modes_.push_back(Sonar::LEFT_TO_LEFT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RIGHT_TO_RIGHT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::LEFT_TO_RIGHT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::RIGHT_TO_LEFT + Sonar::RECEIVE_BOTH);
  // modes_.push_back(Sonar::TRANSMIT_BOTH + Sonar::RECEIVE_BOTH);
  mode_ind_ = 0;

  // in seconds
  send_interval_ = 0.070;
  read_interval_ = 0.070;
  switch_interval_ = 100.00;
  ignore_after_switch_interval_ = 0.100;

  // reset send times
  last_send_time_ = -1;
  last_switch_time_ = -1;
  last_read_time_ = -1;
  
  sameReadingCount = 0;
  prevLeftReading = 0;
  prevRightReading = 0;
  sonarModuleEnabled = true;
}

void SonarModule::processFrame() {
  if (last_send_time_ < 0) {
    initSonar();
    return;
  }

  processSonars();
  processCommands();
}

void SonarModule::processSonars() {

  addDistance(sensors_->sonar_left_[0], 0);
  addDistance(sensors_->sonar_right_[0], 1);
  
//  float epsilon = 0.005;
//
//  // if (frame_info_->seconds_since_start - last_switch_time_ < ignore_after_switch_interval_ - epsilon)
//  //   return;
//  if (frame_info_->seconds_since_start - last_read_time_ < send_interval_ - epsilon)
//    return;
//  last_read_time_ = frame_info_->seconds_since_start;
//
//  static int sameLeft = 0;
//  if(sensors_->sonar_left_[0] == prevLeftReading)
//    sameLeft++;
//  else
//    sameLeft = 0;
//  if(sameLeft < 10)
//    addDistance(sensors_->sonar_left_[0], 0);
//  else
//    addDistance(0, 0);
//
//  static int sameRight = 0;
//  if(sensors_->sonar_right_[0] == prevRightReading)
//    sameRight++;
//  else
//    sameRight = 0;
//  if(sameRight < 10)
//    addDistance(sensors_->sonar_right_[0], 1);
//  else
//    addDistance(0, 1);
//
//  //printf("LEFT: %2.2f, PREV: %2.2f, PROC: %2.2f, SAME COUNT: %i           RIGHT: %2.2f, PREV: %2.2f, PROC: %2.2f, SAME COUNT: %i\n", 
//    //sensors_->sonar_left_[0], prevLeftReading,
//    //processed_sonar_->left_distance_, sameLeft,
//    //sensors_->sonar_right_[0], prevRightReading,
//    //processed_sonar_->right_distance_, sameRight
//  //);
//
//  if (sensors_->sonar_left_[0] != prevLeftReading || sensors_->sonar_right_[0] != prevRightReading) {
//
//    if (!sonarModuleEnabled) {
//      sonarModuleEnabled = true;
//      processed_sonar_->sonar_module_update_ = true;
//      processed_sonar_->sonar_module_enabled_ = true;
//      std::cout << "Sonar module status change to: " << sonarModuleEnabled << std::endl;
//    } else {
//      processed_sonar_->sonar_module_update_ = false;
//    }
//
//    sameReadingCount = 0;
//  } else {
//    if (sonarModuleEnabled) {
//      sameReadingCount++;
//      if (sameReadingCount > SAME_READING_THRESHOLD) {
//        sonarModuleEnabled = false;
//        processed_sonar_->sonar_module_update_ = true;
//        processed_sonar_->sonar_module_enabled_ = false;
//        std::cout << "Sonar module status change to: " << sonarModuleEnabled << std::endl;
//      } 
//    } else {
//      processed_sonar_->sonar_module_update_ = false;
//    }
//  }
//
//  prevLeftReading = sensors_->sonar_left_[0];
//  prevRightReading = sensors_->sonar_right_[0];
//  
//  //std::cout << command_map_[modes_[mode_ind_]] << " " << frame_info_->seconds_since_start << " Le Sonar Reading: " << sensors_->sonar_left_[0] << " "<< sensors_->sonar_right_[0] << " " << std::endl;
//
//  std::cout << "  " << sensors_->sonar_left_[0] << " "<< sensors_->sonar_left_[1] << " "<< sensors_->sonar_left_[2] << " "<< sensors_->sonar_right_[0] << " "    << sensors_->sonar_right_[1] << " " << sensors_->sonar_right_[2] << std::endl;
//
  if (sonarModuleEnabled) {
    filterSonarsFromMedian();
  } else {
    processed_sonar_->on_left_ = false;
    processed_sonar_->on_right_ = false;
    processed_sonar_->on_center_ = false;
    processed_sonar_->left_distance_ = 2.55;
    processed_sonar_->right_distance_ = 2.55;
    processed_sonar_->center_distance_ = 2.55;
  }
}

// This is not actually needed, we let sonars to be always on
void SonarModule::processCommands() {
  // default to not sending commands
  commands_->send_sonar_command_ = false;
  
  // do stuff
  float epsilon = 0.005;
  if (frame_info_->seconds_since_start - last_send_time_ >= send_interval_ - epsilon) {
    commands_->send_sonar_command_ = true;
    last_send_time_ = frame_info_->seconds_since_start;
//     if (frame_info_->seconds_since_start - last_switch_time_ >= switch_interval_ - epsilon) {
// 
//       mode_ind_ = (mode_ind_ + 1) % modes_.size();
// 
//       last_switch_time_ = frame_info_->seconds_since_start;
// //      std::cout << frame_info_->seconds_since_start << " Switching sonar mode to " << modes_[mode_ind_] << std::endl;
//     }

    commands_->sonar_command_ = modes_[mode_ind_];
  }
}

void SonarModule::initSonar() {
  commands_->send_sonar_command_ = true;
  commands_->sonar_command_ = Sonar::RESET;
  last_send_time_ = frame_info_->seconds_since_start + 1.0;
  last_switch_time_ = frame_info_->seconds_since_start + 1.0;
}

void SonarModule::addDistance(float distance, int index) {

  // For confidence based method
  if (distance < max_distance_){ 
    confidence_[index] = (1 - CONF_HISTORY_WEIGHT) + CONF_HISTORY_WEIGHT * confidence_[index];
    distance_[index] = (1 - DIST_HISTORY_WEIGHT) * distance + DIST_HISTORY_WEIGHT * distance_[index];
  } else {
    confidence_[index] = CONF_HISTORY_WEIGHT * confidence_[index];
  }

  // For median based method
  if (index == 0) {
    left_buf_.add(distance);
  } else {
    right_buf_.add(distance);
  }
}

void SonarModule::filterSonarsFromConfidence() {

  processed_sonar_->on_left_ = false;
  processed_sonar_->on_right_ = false;
  processed_sonar_->on_center_ = false;

  if (confidence_[0] > CONFIDENCE) {  // 0 is left
    processed_sonar_->on_left_ = true;
    processed_sonar_->left_distance_ = distance_[0];
  } 
  if (confidence_[1] > CONFIDENCE) {  // 1 is right
    processed_sonar_->on_right_ = true;
    processed_sonar_->right_distance_ = distance_[1];
  } 

  processed_sonar_->on_center_ = processed_sonar_->on_left_
      && processed_sonar_->on_right_
      && fabs(processed_sonar_->left_distance_ - processed_sonar_->right_distance_) < lr_distance_diff_;
  if (processed_sonar_->on_center_) {
    processed_sonar_->on_left_ = processed_sonar_->on_right_ = false;
    processed_sonar_->center_distance_ = (processed_sonar_->left_distance_ + processed_sonar_->right_distance_) / 2;
    //std::cout << " Center: " << processed_sonar_->center_distance_ << std::endl;
  } else {
    /*
    bool endLine = false;
    if (processed_sonar_->on_left_) {
      std::cout << " Left: " << processed_sonar_->left_distance_;
      endLine = true;
    }
    if (processed_sonar_->on_right_) {
      std::cout << " Right: " << processed_sonar_->right_distance_;
      endLine = true;
    }
    if (endLine)
      std::cout << std::endl;
    */
  }

}
void SonarModule::filterSonarsFromMedian() {

  processed_sonar_->on_left_ = false;
  processed_sonar_->on_right_ = false;
  processed_sonar_->on_center_ = false;

  float left_distance_ = left_buf_.getMedian();
  processed_sonar_->left_distance_ = left_distance_;
  if (left_distance_ > min_distance_ && left_distance_ <= max_distance_) {  // 0 is left
    processed_sonar_->on_left_ = true;
  } 
  float right_distance_ = right_buf_.getMedian();
  processed_sonar_->right_distance_ = right_distance_;
  if (right_distance_ > min_distance_ && right_distance_<= max_distance_) {  // 1 is right
    processed_sonar_->on_right_ = true;
  } 

  processed_sonar_->on_center_ = processed_sonar_->on_left_
      && processed_sonar_->on_right_
      && fabs(processed_sonar_->left_distance_ - processed_sonar_->right_distance_) < lr_distance_diff_;

  if (processed_sonar_->on_center_) {
    processed_sonar_->on_left_ = processed_sonar_->on_right_ = false;
    processed_sonar_->center_distance_ = (processed_sonar_->left_distance_ + processed_sonar_->right_distance_) / 2;
  } else {
    processed_sonar_->center_distance_ = 2.55;
    // bool endLine = false;
    // if (processed_sonar_->on_left_) {
    //   std::cout << " Left: " << processed_sonar_->left_distance_;
    //   endLine = true;
    // }
    // if (processed_sonar_->on_right_) {
    //   std::cout << " Right: " << processed_sonar_->right_distance_;
    //   endLine = true;
    // }
    // if (endLine)
    //   std::cout << std::endl;
  }

//  std::cout << "right: " << processed_sonar_->on_right_ << ", left: " << processed_sonar_->on_left_ << ", center: " << processed_sonar_->on_center_ << std::endl;
//  std::cout << "right dist: " << processed_sonar_->right_distance_ << ", left dist: " << processed_sonar_->left_distance_ << ", center dist: " << processed_sonar_->center_distance_ << std::endl;

}
