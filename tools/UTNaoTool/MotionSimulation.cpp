#include "MotionSimulation.h"


// memory
#include <memory/MemoryFrame.h>
#include <memory/BodyModelBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/GraphableBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WalkEngineBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/WalkParamBlock.h>
#include <memory/KickEngineBlock.h>
#include <memory/KickModuleBlock.h>
#include <memory/KickParamBlock.h>
#include <memory/KickRequestBlock.h>
#include <memory/WalkInfoBlock.h>

#include <kinematics/ForwardKinematics.h>


#include <stdlib.h>

// walk engine
#include <motion/WalkModule.h>
#include <motion/MotionModule.h>
#include <motion/GetupModule.h>

#include <MotionCore.h>
#include <VisionCore.h>
#include <kinematics/KinematicsModule.h>

MotionSimulation::MotionSimulation(MemoryFrame* mem){
  
  // init walk engine
  //cout << "init motion core" << endl << flush;
  core_ = new MotionCore(CORE_TOOL,false,0,1);
  vcore_ =new VisionCore(CORE_TOOL,false,0,1);
  

  updateMemoryBlocks();
  angle_time = -1;
  use_com_kick_ = core_->use_com_kick_;
  getUpSide = 0;

  // fill in current joint angles and walk paramsfrom mem
  // and processed sensors
  JointBlock* origJointAngles = NULL;
  WalkParamBlock* origWalkParam = NULL;
  SensorBlock* origSensors = NULL;

  if (mem != NULL){
    mem->getBlockByName(origJointAngles,"processed_joint_angles",false);
    mem->getBlockByName(origWalkParam,"walk_param",false);
    mem->getBlockByName(origSensors,"processed_sensors",false);
    
    if (origJointAngles != NULL){
      *joint_angles_ = *origJointAngles;
    }
    if (origWalkParam != NULL){
      *walk_params_ = *origWalkParam;
    }
    
    if (origSensors != NULL){
      *sensors_ = *origSensors;
    }
  }

  if (origWalkParam == NULL){
    cout << endl << "ERROR: WALK SIM requires log with walk_param!!!" << endl << endl;
  }


  getLuaParameters();

  kick_request_->ball_image_center_x_ = 320;
  kick_request_->ball_image_center_y_ = 300;
  kick_request_->ball_seen_ = true;
  kick_request_->kick_running_ = false;

  //std::cout << "sensors: " << std::endl;
  //for (int i = 0; i < NUM_SENSORS; i++)
    //std::cout << SensorNames[i] << " " << sensors_->values_[i] << std::endl;
  
  //std::cout << "....ORIG...." << std::endl;
  //for (int i = 0; i < NUM_JOINTS; i++)
    //std::cout << JointNames[i] << " " << joint_angles_->values_[i] << std::endl;
  //std::cout << "............" << std::endl;
}
MotionSimulation::~MotionSimulation(){
  delete core_;
  delete memory_;
  core_ = NULL;
  memory_ = NULL;
}


MemoryFrame* MotionSimulation::getMemory(){
  return memory_;
}

void MotionSimulation::getLuaParameters() {
  // get interpreter parameters for kick
  VisionCore vision_core_(CORE_TOOL_NO_VISION,false,0,1);
  vision_core_.interpreter_->start();
  vision_core_.processVisionFrame();
  KickParamBlock* kickParams = NULL;
  vision_core_.memory_->getBlockByName(kickParams,"vision_kick_params");
  *kick_params_ = *kickParams;
}

void MotionSimulation::updateMemoryBlocks(){
  memory_ = &(core_->memory_);
  
  memory_->getOrAddBlockByName(body_model_,"body_model");
  memory_->getOrAddBlockByName(frame_info_,"frame_info");
  memory_->getOrAddBlockByName(graph_,"graphable");
  memory_->getOrAddBlockByName(commands_,"processed_joint_commands");
  memory_->getOrAddBlockByName(joint_angles_,"processed_joint_angles");
  memory_->getOrAddBlockByName(odometry_,"odometry");
  memory_->getOrAddBlockByName(sensors_,"processed_sensors");
  memory_->getOrAddBlockByName(walk_engine_mem_,"walk_engine");
  memory_->getOrAddBlockByName(walk_request_,"walk_request");
  memory_->getOrAddBlockByName(kick_engine_mem_,"kick_engine");
  memory_->getOrAddBlockByName(kick_module_mem_,"kick_module");
  memory_->getOrAddBlockByName(kick_request_,"kick_request");
  memory_->getOrAddBlockByName(walk_info_,"walk_info");
  //cout << "try to get core walk param" << endl << flush;
  memory_->getOrAddBlockByName(walk_params_,"walk_param");
  memory_->getOrAddBlockByName(kick_params_,"kick_params");

  walk_request_->target_point_ = Vector2<float>(500,30);
  sensors_->values_[accelZ] = 9.8;
}

bool MotionSimulation::processFrame(){

  updateInputs();

  // call walk engine process frame
  core_->kinematics_->calculatePose();

  //if (core_->walk_ != NULL)
    //core_->walk_->processFrame();
  //if (core_->htwk_walk_ != NULL)
    //core_->htwk_walk_->processFrame();
  //if (core_->bh_walk_ != NULL)
    //core_->bh_walk_->processFrame();

  // override commands with getup if necessary
  if (core_->getup_->isGettingUp() || walk_request_->motion_==WalkRequestBlock::FALLING){
    // possibly init get up
    if (!core_->getup_->isGettingUp())
      core_->getup_->initGetup();
    core_->getup_->processFrame();
  } 

  return updateOutputs();

}

void MotionSimulation::updateInputs(){
  
  float timeStep = 10; // in ms
  // change frame info (100 Hz)
  frame_info_->seconds_since_start += (timeStep/1000.0);
  frame_info_->frame_id++;
  frame_info_->source = MEMORY_ROBOT;

  // maybe update tilt/roll
  // maybe update walk request

  // update joint angles to based on saved joint commands
  if (angle_time > 0){
    for (int i = 0; i < NUM_JOINTS; i++){
      if (isnan(angle_commands[i])){
        //cout << "Commanded angle for joint " << i << " is NAN" << endl;
        continue;
      }
      // get difference
      float diff = angle_commands[i] - joint_angles_->values_[i];
      // pct we'll move (assuming 100 Hz - 10ms)
      float pctMove = timeStep / angle_time;
      if (pctMove > 1.0) pctMove = 1.0;
      //cout << "Joint " << i << " Command: " << commands_->angles_[i]*RAD_T_DEG << " Current: " << joint_angles_->values_[i]*RAD_T_DEG << " diff: " << diff*RAD_T_DEG << " time: " << commands_->body_angle_time_ << " pctMove: " << pctMove << endl;
      joint_angles_->values_[i] += pctMove * diff;
    }

    // subtract off 1 frame of time from angle time (so if it was a command w/ 1000 ms interpolation, now we interp over the remaining 990 ms
    // 10 ms / 100 Hz
    angle_time -= timeStep;
  }

  // figure out body tilt roll based on angles
  //bool stanceLeg = true;
  //// if there's a kick, use stance leg from that
  //if (kick_request_->kick_running_){
    //if (use_com_kick_) {
      //if (kick_module_mem_->swing_leg_ == Kick::RIGHT)
        //stanceLeg = true;
      //else 
        //stanceLeg = false;
    //} else {
      //if (kick_engine_mem_->leg_ == Kick::RIGHT)
        //stanceLeg = true;
      //else 
        //stanceLeg = false;
    //}
  //} else {
    //if (walk_engine_mem_->step_current_.is_left_foot_)
      //stanceLeg = true;
    //else
      //stanceLeg = false;
  //}

  //std::cout << "----JOINTS----" << std::endl;
  //for (int i = 0; i < NUM_JOINTS; i++)
    //std::cout << JointNames[i] << " " << joint_angles_->values_[i] << std::endl;
  //std::cout << "--------------" << std::endl;

  //std::cout << "____DIMS____" << std::endl;
  //for (int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++)
    //std::cout << i << " " << dimensions_.values_[i] << std::endl;
  //std::cout << "____________" << std::endl;

  TiltRoll tr = ForwardKinematics::calculateTiltRollFromLeg(true, &joint_angles_->values_[0], dimensions_);
    
  // set in sensors
  sensors_->values_[angleX] = tr.roll_;
  sensors_->values_[angleY] = tr.tilt_;
  
  //std::cout << sensors_->values_[angleX] << " " << sensors_->values_[angleY] << std::endl;

  
  walk_request_->tilt_fallen_counter_ = getUpSide;
  
  
  // put on front
  if (walk_request_->tilt_fallen_counter_ > 0){
    sensors_->values_[angleY] = M_PI/2.0;
    sensors_->values_[angleX] = 0;
  
  } 
  // put on back
  else if (walk_request_->tilt_fallen_counter_ < 0) {
    sensors_->values_[angleY] = -M_PI/2.0;
    sensors_->values_[angleX] = 0;

  }
  
  walk_info_->instability_ = 0;
  walk_info_->stabilizer_off_threshold_ = 100;

  // fake localization using odometry
  if (frame_info_->frame_id % 4 == 0){
    // move target point closer by odometry amount
    Pose2D target(0,0,0);
    target = walk_request_->target_point_;
    walk_request_->target_point_ = target.globalToRelative(odometry_->displacement).translation;

    // rotate target heading by odometry as well
    walk_request_->rotate_heading_ -= odometry_->displacement.rotation;

    // vision used odom, reset
    odometry_->displacement = Pose2D(0,0,0);
  }

  // reset any kick requests
  if (use_com_kick_) {
    if (kick_request_->kick_type_ == kick_engine_mem_->type_)
      kick_request_->kick_type_ = Kick::NO_KICK;
  } else {
    if (kick_request_->kick_type_ == kick_module_mem_->kick_type_)
      kick_request_->kick_type_ = Kick::NO_KICK;
  }

}

bool MotionSimulation::updateOutputs(){
  
  // if there's a new command...
  // fill in our copy of commands
  if (commands_->send_body_angles_){
    for (int i = 0; i < NUM_JOINTS; i++){
      if (isnan(commands_->angles_[i])){
        //cout << "Commanded angle for joint " << i << " is NAN" << endl;
        continue;
      }
      angle_commands[i] = commands_->angles_[i];
      angle_time = commands_->body_angle_time_;
    }
  }

  return false;
}


void MotionSimulation::setWalkRequest(bool walk, float x, float y, float rot){
  if (walk){
    walk_request_->setWalk(x,y,rot);
  } else {
    walk_request_->stand();
  }
}

void MotionSimulation::setKickRequest(Kick::Type kickType, Kick::Leg leg, float kickHeading, float kickDistance){
  walk_request_->noWalk();
  kick_request_->set(kickType, leg, kickHeading, kickDistance);
  kick_request_->ball_seen_ = true;
  kick_request_->kick_running_ = true;
}


void MotionSimulation::incrWalkRequest(bool walk, float x, float y, float rot){
  if (walk){
    walk_request_->motion_ =WalkRequestBlock::WALK;
    walk_request_->percentage_speed_ = true;
    walk_request_->pedantic_walk_ = false;
    walk_request_->walk_to_target_ = false;
    walk_request_->rotate_around_target_ = false;

    walk_request_->speed_.translation.x += x;
    walk_request_->speed_.translation.y += y;
    walk_request_->speed_.rotation += rot;
  }
}
