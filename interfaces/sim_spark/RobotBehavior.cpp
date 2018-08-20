#include "RobotBehavior.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cctype>
#include <exception>
#include <ctime>

#include <common/InterfaceInfo.h>
#include <boost/math/distributions/normal.hpp>

#include "encode.h"

RobotBehavior::RobotBehavior(const std::string teamName, int uNum)
{
  cout << "Constructing of Nao Behavior" << endl;
  
  agentTeamName=teamName;
  agentUNum=uNum;

  int team_num = atoi(agentTeamName.c_str());

  if (team_num != 0 && team_num != 1){
    cout << "Invalid team num (0,1): " << team_num << endl;
    exit(0);
  }
  cout << "team num is " << team_num << endl;

  init_ = false;
  init_beam_ = false;

  memory_ = new MemoryFrame(true,MemoryOwner::INTERFACE,team_num,agentUNum,true);

  FrameInfoBlock *block;
  memory_->getOrAddBlockByName(block,"vision_frame_info",MemoryOwner::VISION);
  block->source = MEMORY_SIM;

  memory_->getOrAddBlockByName(frame_info_,"frame_info");
  memory_->getOrAddBlockByName(vision_frame_info_,"raw_vision_frame_info",MemoryOwner::IMAGE_CAPTURE);
  vision_frame_info_->log_block = false;
  memory_->getOrAddBlockByName(camera_info_,"raw_camera_info",MemoryOwner::IMAGE_CAPTURE);
  frame_info_->source = MEMORY_SIM; // set to simulaor
  vision_frame_info_->source = MEMORY_SIM;
   
  memory_->getOrAddBlockByName(raw_sensor_block_,"raw_sensors");
  memory_->getOrAddBlockByName(raw_joint_angles_,"raw_joint_angles");
  memory_->getOrAddBlockByName(raw_joint_commands_,"raw_joint_commands");
  memory_->getOrAddBlockByName(sim_effectors_,"sim_effectors");
  memory_->getOrAddBlockByName(raw_image_,"raw_image",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(sim_truth_data_,"sim_truth_data",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(team_packets_,"team_packets",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(game_state_,"game_state",MemoryOwner::IMAGE_CAPTURE);
  
  memory_->getOrAddBlockByName(robot_info_,"robot_info",MemoryOwner::SHARED);
  memory_->getOrAddBlockByName(robot_state_,"robot_state",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(robot_vision_,"robot_vision",MemoryOwner::IMAGE_CAPTURE);

  std::cout << "SETTING ROBOT NUMBER (WO_SELF) = " << uNum << std::endl;
  robot_state_->WO_SELF = agentUNum; // set our player number
  robot_state_->team_ = team_num;
  robot_state_->role_ = agentUNum;
  
  memory_->getOrAddBlockByName(world_objects_,"world_objects",MemoryOwner::IMAGE_CAPTURE);
  world_objects_->init(robot_state_->team_);
  
  // set up locks
  cleanLock(Lock::getLockName(memory_,LOCK_MOTION));
  motion_lock_ = new Lock(Lock::getLockName(memory_,LOCK_MOTION));
  cleanLock(Lock::getLockName(memory_,LOCK_VISION));
  vision_lock_ = new Lock(Lock::getLockName(memory_,LOCK_VISION));
  
  parser_ = new Parser(this,vision_lock_); //vision_frame_info_,frame_info_,raw_joint_angles_,raw_sensors_,raw_image_,sim_truth_data_,world_objects_,vision_lock_,robot_state_, team_packets_);

}

RobotBehavior::~RobotBehavior(){
  delete memory_;
  delete motion_lock_;
}

string RobotBehavior::Init(){
  return "(scene rsg/agent/nao/nao.rsg)";
}

string RobotBehavior::Think(const std::string& message) {
  string action;

  if (!init_){
    random_.reset(std::time(0));
    init_ = true;
    stringstream ss;
    ss << "(init (unum " << agentUNum << ")(teamname " << agentTeamName << "))";
    //cout << "(init (unum " << agentUNum << ")(teamname " << agentTeamName << "))" << endl;
    action = ss.str();
    return action;
  }
  
  if (!init_beam_) {
    init_beam_ = true;
    stringstream ss;
    if (agentUNum==1) {
      ss << "(beam " << -2.5 << " " << 0 << " " << 0 << ")";
    } else if (agentUNum==2) {
      ss << "(beam " << -1.5 << " " << 1.5 << " " << 0 << ")";
    } else if (agentUNum==3) {
      ss << "(beam " << -1.5 << " " << -1.5 << " " << 0 << ")";
    } else if (agentUNum==4) {
      ss << "(beam " << -0.25 << " " << 0.0 << " " << 0 << ")";
    }
    action = ss.str();
    return action;
  }

  bool parseSuccess = parser_->parse(message);
  calculateAngles();
  
  bool res;
  res = motion_lock_->timed_lock(9);
  if (!res)
    std::cout << "WARNING: Didn't acquire lock, but continuing anyway" << std::endl << std::flush;
  
  //frame_info_->seconds_since_start= frame_info_->seconds_since_start; // already handled
  frame_info_->frame_id++;
  
  raw_joint_angles_->values_[RHipYawPitch] = raw_joint_angles_->values_[RHipYawPitch]; 
  
  //preProcessJoints();  // Apply the correct sign to the joint angles 

  if (res) {
    motion_lock_->notify_one();
    bool res2 = motion_lock_->timed_wait(9);
    //if (!res2)
      //std::cout << "WAIT TIMED OUT, continuing anyway" << std::endl;
  }

  //postProcessJoints(); // Flip the joint angles back
  action = composeAction();

  if (res)
    motion_lock_->unlock();
  motion_lock_->notify_one();
  return action;
}

void RobotBehavior::calculateAngles() {
  
  float  accX = raw_sensor_block_->values_[accelX];
  float  accY = raw_sensor_block_->values_[accelY];
  float  accZ = raw_sensor_block_->values_[accelZ];

  raw_sensor_block_->values_[angleX] = atan2(accY,accZ);
  raw_sensor_block_->values_[angleY] = -atan2(accX,accZ);

  //raw_sensors_->values_[gyroX] = 0; // = 1000000.0;
  //raw_sensors_->values_[gyroY] = 0; //= 1000000.0;
}

string RobotBehavior::composeAction(){
    
  stringstream ss("");
  
  ss << "(he1 " << computeTorque(HeadYaw) << ")";
  ss << "(he2 " << computeTorque(HeadPitch) << ")";
  
  ss << "(lae1 " << computeTorque(LShoulderPitch) << ")";
  ss << "(lae2 " << computeTorque(LShoulderRoll) << ")";
  ss << "(lae3 " << computeTorque(LElbowYaw) << ")";
  ss << "(lae4 " << computeTorque(LElbowRoll) << ")";
  
  ss << "(rae1 " << computeTorque(RShoulderPitch) << ")";
  ss << "(rae2 " << computeTorque(RShoulderRoll) << ")";
  ss << "(rae3 " << computeTorque(RElbowYaw) << ")";
  ss << "(rae4 " << computeTorque(RElbowRoll) << ")";

  ss << "(lle1 " << computeTorque(LHipYawPitch) << ")";
  ss << "(lle2 " << computeTorque(LHipRoll) << ")";
  ss << "(lle3 " << computeTorque(LHipPitch) << ")";
  ss << "(lle4 " << computeTorque(LKneePitch) << ")";
  ss << "(lle5 " << computeTorque(LAnklePitch) << ")";
  ss << "(lle6 " << computeTorque(LAnkleRoll) << ")";
  
  ss << "(rle1 " << computeTorque(RHipYawPitch) << ")";
  ss << "(rle2 " << computeTorque(RHipRoll) << ")";
  ss << "(rle3 " << computeTorque(RHipPitch) << ")";
  ss << "(rle4 " << computeTorque(RKneePitch) << ")";
  ss << "(rle5 " << computeTorque(RAnklePitch) << ")";
  ss << "(rle6 " << computeTorque(RAnkleRoll) << ")";

  TeamPacket* tp = &(team_packets_->tp[robot_state_->WO_SELF]);
  if (vision_frame_info_->frame_id % 5 == 0 && (tp->robotNumber>0 && tp->robotNumber<5)) {
    // show that we sent 
    team_packets_->frameReceived[robot_state_->WO_SELF] = frame_info_->frame_id;
    base64::Encoder theBase64Encoder; // Convert message to Base64 so it doesn't get corrupted by the simulator
    std::string output = theBase64Encoder.encode((const char*)tp,sizeof(TeamPacket));
    ss << "(say "<< output << ")";
  }
  return ss.str();
}

double RobotBehavior::computeTorque(const int &effectorID) {
  SimEffector* effector = &(sim_effectors_->effector_[effectorID]);

  effector->k1_ = 0.2; //1.0;
  effector->k3_ = 0.05; //1.0;

  // this updates what we sensed, and how much time until we want to reach target angle
  effector->updateSensorAngle(RAD_T_DEG*raw_joint_angles_->values_[effectorID]);
  //std::cout << "sensed angle at " << effector->current_angle_ << " time: " << effector->target_time_ << std::endl;

  // if new command, update target and time
  if (effectorID == HeadPitch && raw_joint_commands_->send_head_pitch_angle_){
    effector->updateCommand(RAD_T_DEG*raw_joint_commands_->angles_[effectorID], raw_joint_commands_->head_pitch_angle_time_, raw_joint_commands_->head_pitch_angle_change_);
  } else if (effectorID == HeadYaw && raw_joint_commands_->send_head_yaw_angle_){
    effector->updateCommand(RAD_T_DEG*raw_joint_commands_->angles_[effectorID], raw_joint_commands_->head_yaw_angle_time_, raw_joint_commands_->head_yaw_angle_change_);
  } else if (effectorID != HeadPitch && effectorID != HeadYaw && raw_joint_commands_->send_body_angles_){
    effector->updateCommand(RAD_T_DEG*raw_joint_commands_->angles_[effectorID], raw_joint_commands_->body_angle_time_, false);
  } 

  // calculate immediate target angle (based on interpolation to target point)
  float angle_range = effector->target_angle_ - effector->current_angle_;
  float frac = 20.0 / effector->target_time_;
  if (frac < 0 || frac > 1) frac = 1;
  float change = frac * angle_range;
  float immediate_target = effector->current_angle_ + change;
  //std::cout << "immediate target " << immediate_target << " range: " << angle_range << " time: " << effector->target_time_ << " frac: " << frac << " change: " << change << std::endl;

  effector->setImmediateTargetAngle(immediate_target);

  // update errors
  effector->updateErrors();
  //std::cout << " curr err: " << effector->current_error_ << " cum err: " << effector->cumulative_error_ << " prev err: " << effector->previous_error_ << std::endl;

  double torque = effector->k1_ * effector->current_error_;
  torque += effector->k2_ * effector->cumulative_error_;
  torque += effector->k3_ * (effector->current_error_ - effector->previous_error_);

  float expected_torque = effector->scale_ * torque;

  float r = 1.0; // no noise
  //float r = random_.normal(0.7,0.15);
  
  //std::cout << "torque: " << r << " " << expected_torque << " " << r*expected_torque << std::endl; 
  return r*expected_torque;
}

