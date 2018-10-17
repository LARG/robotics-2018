#include "MotionCore.h"

#include <memory/BodyModelBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/KickRequestBlock.h>
#include <memory/SensorBlock.h>
#include <memory/KickParamBlock.h>
#include <memory/WalkParamBlock.h>
#include <memory/ALWalkParamBlock.h>
#include <memory/WalkInfoBlock.h>
#include <memory/RobotInfoBlock.h>
//#include <memory/WorldObjectBlock.h>//for rswalk2014
#include <memory/RobotStateBlock.h>
#include <memory/BehaviorBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/WalkResponseBlock.h>
#include <memory/ProcessedSonarBlock.h>

#include <kinematics/KinematicsModule.h>
#include <motion/KickModule.h>
#include <motion/MotionModule.h>
#include <motion/GetupModule.h>
#include <motion/rswalk2014/RSWalkModule2014.h>
#include <motion/SpecialMotionModule.h>
#include <sensor/SensorModule.h>
#include <sonar/SonarModule.h>

#include <common/InterfaceInfo.h>
#include <math/Geometry.h>
#include <math/Pose2D.h>

#include <common/Calibration.h>
#include <common/RobotCalibration.h>
#include <common/Kicks.h>

#include <boost/interprocess/sync/named_mutex.hpp>
#include <common/RobotConfig.h>

MotionCore *MotionCore::inst_ = NULL;

MotionCore::MotionCore (CoreType type, bool use_shared_memory,int team_num, int player_num):
  memory_(use_shared_memory,MemoryOwner::MOTION,team_num,player_num,false),
  type_(type),
  last_frame_processed_(0),
  use_com_kick_(true),
  kinematics_(NULL),
  kick_(NULL),
  motion_(NULL),
  sensor_(NULL),
  sonar_(NULL),
  getup_(NULL),
  specialM_(NULL),
  walk_(NULL),
  textlog_(std::make_unique<TextLogger>()),
  sync_joint_angles_(NULL),
  sync_kick_request_(NULL),
  sync_odometry_(NULL),
  sync_sensors_(NULL),
  sync_walk_request_(NULL),
  sync_walk_response_(NULL),
  sync_joint_commands_(NULL),
  sync_processed_sonar_(NULL),
  sync_kick_params_(NULL),
  sync_walk_param_(NULL),
  sync_al_walk_param_(NULL),
  sync_walk_info_(NULL),
  fps_frames_processed_(0),
  calibration_(NULL)
{
  init();
}

MotionCore::~MotionCore() {
  // clean up the modules
  if (kinematics_ != NULL)
    delete kinematics_;
  if (kick_ != NULL)
    delete kick_;
  if (motion_ != NULL)
    delete motion_;
  if (sensor_ != NULL)
    delete sensor_;
  if (sonar_ != NULL)
    delete sonar_;
  if (walk_ != NULL)
    delete walk_;
  //if (htwk_walk_ != NULL)
    //delete htwk_walk_;
  if (walk_ != NULL)
    delete walk_;
}

void MotionCore::loadCalibration() {
  static bool attempted = false;
  if(attempted) return;
  attempted = true;
  RobotConfig config;
  config.loadFromFile(memory_.data_path_ + "/config.yaml");
  auto calfile = util::ssprintf("%s/%02i_calibration.cal", memory_.data_path_, config.robot_id);
  RobotCalibration cal;
  bool loaded = cal.loadFromFile(calfile);
  if(loaded) {
    calibration_ = new RobotCalibration(cal);
    printf("Loaded calibration: %s\n", calfile.c_str());
  }
  else {
    std::cout << "ERROR: No robot calibration found\n";
  }
}


void MotionCore::init() {
  if (type_ == CORE_INIT) {
    FrameInfoBlock *frame_info;
    memory_.getBlockByName(frame_info,"frame_info");
    if (frame_info->source == MEMORY_SIM) {
      std::cout << "MOTION CORE: SIM" << std::endl;
      type_ = CORE_SIM;
    } else if (frame_info->source == MEMORY_ROBOT) {
      std::cout << "MOTION CORE: ROBOT" << std::endl;
      type_ = CORE_ROBOT;
    } else {
      std::cerr << "Unknown memory type when init vision core" << std::endl;
      exit(1);
    }
  }

  inst_ = this;

  setMemoryVariables();

  initMemory();
  initModules();

  fps_time_ = frame_info_->seconds_since_start;
  time_motion_started_ = frame_info_->seconds_since_start;
  walk_request_->walk_type_ = INVALID_WALK;
  loadCalibration();
}

bool MotionCore::alreadyProcessedFrame() {
  if (frame_info_->frame_id <=last_frame_processed_) {
    return true;
  } else {
    return false;
  }
}

void MotionCore::processMotionFrame() {
  auto& frame_id = frame_info_->frame_id;
  if (alreadyProcessedFrame()) {
    //std::cout << "processMotionFrame, skipping frame " << frame_info_->frame_id << std::endl;
    return;
  }

  if (frame_id > last_frame_processed_ + 1) {
    std::cout << "Skipped a frame: went from " << last_frame_processed_ << " to " << frame_id << std::endl;
  }
  // frame rate
  double time_passed = frame_info_->seconds_since_start - fps_time_;
  fps_frames_processed_++;
  if (time_passed >= 10.0) {
    std::cout << "MOTION FRAME RATE: " << fps_frames_processed_ / time_passed << std::endl;
    fps_frames_processed_ = 0;
    fps_time_ = frame_info_->seconds_since_start;
  }

  // actually do stuff
  processSensorUpdate();
  // check if the get up wants us to stand
  if (getup_->needsStand()) {
    walk_request_->stand();
  }

  // kicks need to be before walk, so that they can change the walk request
  if (use_com_kick_)
    kick_->processFrame();
  else
    motion_->processFrame();


  if (walk_ != NULL) {
    walk_->processFrame();
  }



  // override commands with getup if necessary
  if (getup_->isGettingUp() or walk_request_->motion_ == WalkRequestBlock::FALLING) {
    // possibly init get up
    if (not getup_->isGettingUp())
      getup_->initGetup();
    getup_->processFrame();
  }

  last_frame_processed_ = frame_id;
}

void MotionCore::processSensorUpdate() {
  sensor_->processSensors();
  kinematics_->calculatePose();
  sonar_->processFrame();
}

void MotionCore::initModules() {
  kinematics_ = new KinematicsModule();
  kinematics_->init(&memory_,textlog_.get());

  kick_ = new KickModule();
  kick_->init(&memory_,textlog_.get());

  motion_ = new MotionModule();
  motion_->init(&memory_,textlog_.get());

  sensor_ = new SensorModule();
  sensor_->init(&memory_,textlog_.get());
  sonar_ = new SonarModule();
  sonar_->init(&memory_,textlog_.get());
  getup_ = new GetupModule();
  getup_->init(&memory_,textlog_.get());
  //specialM_=new SpecialMotionModule();
  //specialM_->init(&memory_,textlog_.get());
}

void MotionCore::initWalkEngine() {
  if(walk_request_->walk_type_ == INVALID_WALK) return;
  if(walk_ != NULL) delete walk_;
  switch (walk_request_->walk_type_) {
    //RS2014
    case RUNSWIFT2014_WALK:
      printf("Loaded RS2014 walk.\n");
      walk_ = new RSWalkModule2014();
      walk_->init(&memory_,textlog_.get());
      break;
    default:
      printf("No walk loaded\n");
      walk_ = NULL;
      break;
  }
}

void MotionCore::initMemory() {
  // create all the modules that are used
  //MemorySource mem_source = MEMORY_SIM;
  //if (type_ == CORE_ROBOT)
    //mem_source = MEMORY_ROBOT;

  //Add required memory blocks
  memory_.addBlockByName("body_model");
  memory_.addBlockByName("graphable");
  memory_.addBlockByName("kick_engine");
  memory_.addBlockByName("walk_engine");
  memory_.addBlockByName("odometry");
  //memory_.addBlockByName("sonar");

  memory_.getOrAddBlockByName(frame_info_,"frame_info");
  // joint angles
  memory_.getBlockByName(raw_joint_angles_,"raw_joint_angles");
  memory_.getOrAddBlockByName(processed_joint_angles_,"processed_joint_angles");
  // commands
  memory_.getBlockByName(raw_joint_commands_,"raw_joint_commands");
  memory_.getOrAddBlockByName(processed_joint_commands_,"processed_joint_commands");
  // sensors
  memory_.getBlockByName(raw_sensors_,"raw_sensors");
  memory_.getOrAddBlockByName(processed_sensors_,"processed_sensors");

  memory_.getOrAddBlockByName(body_model_,"body_model");
  memory_.getOrAddBlockByName(walk_request_,"walk_request");
  memory_.getOrAddBlockByName(walk_response_,"walk_response");
  memory_.getOrAddBlockByName(kick_params_,"kick_params");
  memory_.getOrAddBlockByName(walk_param_,"walk_param");
  memory_.getOrAddBlockByName(al_walk_param_,"al_walk_param");
  memory_.getOrAddBlockByName(walk_info_,"walk_info");

  memory_.getOrAddBlockByName(kick_request_,"kick_request");
  memory_.getOrAddBlockByName(odometry_,"odometry");
  memory_.getOrAddBlockByName(processed_sonar_,"processed_sonar");

  memory_.getOrAddBlockByName(robot_info_,"robot_info",MemoryOwner::SHARED);
  //memory_.getOrAddBlockByName(world_objects_,"world_objects",MemoryOwner::SHARED);//for rswalk2014
  memory_.getOrAddBlockByName(robot_state_,"robot_state",MemoryOwner::SHARED);

  // synchronized blocks - the true means remove any existing locks
  memory_.getOrAddBlockByName(sync_body_model_,"sync_body_model",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_joint_angles_,"sync_joint_angles",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_kick_request_,"sync_kick_request",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_odometry_,"sync_odometry",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_sensors_,"sync_sensors",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_walk_request_,"sync_walk_request",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_walk_response_,"sync_walk_response",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_joint_commands_,"sync_joint_commands",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_processed_sonar_,"sync_processed_sonar",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_kick_params_,"sync_kick_params",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_walk_param_,"sync_walk_param",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_al_walk_param_,"sync_al_walk_param",MemoryOwner::SYNC);
  memory_.getOrAddBlockByName(sync_walk_info_,"sync_walk_info",MemoryOwner::SYNC);

  // Read configuration file and place appropriate values in memory
  if (type_ == CORE_ROBOT) {
    Calibration calibration;
    std::cout << "Reading calibration file: " << memory_.data_path_ + "/calibration.txt" << std::endl;
    if (calibration.readFromFile(memory_.data_path_ + "/calibration.txt")) {
      // Disabled these because of new calibration system - JM 05/08/13
      //robot_info_->dimensions_.setCameraParameters(DEG_T_RAD*calibration.tilt_bottom_cam_, DEG_T_RAD*calibration.roll_bottom_cam_, DEG_T_RAD*calibration.tilt_top_cam_, DEG_T_RAD*calibration.roll_top_cam_);
      //robot_info_->dimensions_.setHeadOffsets(DEG_T_RAD * calibration.head_tilt_offset_, DEG_T_RAD * calibration.head_pan_offset_);
    }
  }

  // print out all the memory blocks we're using
  std::vector<std::string> memory_block_names;
  memory_.getBlockNames(memory_block_names,false);
  std::cout << "INITIAL MEMORY BLOCKS:" << std::endl;
  for (unsigned int i = 0; i < memory_block_names.size(); i++)
    std::cout << memory_block_names[i] << std::endl;
  std::cout << "--------------" << std::endl;

  textlog_->setFrameInfo(frame_info_);
}

void MotionCore::enableTextLogging(const char *filename) {
  if (filename) {
    textlog_->open(filename);
  } else {
    textlog_->open("motion", true);
  }
}

void MotionCore::disableTextLogging() {
  textlog_->close();
}

void MotionCore::setMemoryVariables() {

  // Set the data path for the data folder
  switch (type_) {
  case CORE_ROBOT:
    memory_.data_path_ = "/home/nao/data";
    break;
  case CORE_SIM:
  case CORE_TOOL:
  default:
    memory_.data_path_ = util::ssprintf("%s/data", util::env("NAO_HOME"));
    break;
  }

  // Pass the coretype to the memory so that it can be accessed by different modules
  memory_.core_type_ = type_;
}

void MotionCore::preProcess() {
  const int *signs;
  if (frame_info_->source == MEMORY_ROBOT)
    signs = robot_joint_signs;
  else
    signs = spark_joint_signs;

  // apply signs to joint angles
  for (int i=0; i<NUM_JOINTS; i++) {
    processed_joint_angles_->prevValues_[i] = processed_joint_angles_->values_[i];
    processed_joint_angles_->values_[i] = signs[i] * raw_joint_angles_->values_[i];
    processed_joint_angles_->stiffness_[i] = raw_joint_angles_->stiffness_[i];
  }

  // handle head offsets for reading
  processed_joint_angles_->values_[HeadTilt] += robot_info_->dimensions_.values_[RobotDimensions::headTiltOffset];
  processed_joint_angles_->values_[HeadYaw] += robot_info_->dimensions_.values_[RobotDimensions::headPanOffset];
  
  if(calibration_ && calibration_->enabled) {
    calibration_->applyJoints(processed_joint_angles_->values_.data());
  }
}

void MotionCore::postProcess() {
  // process
  const int *signs;
  if (frame_info_->source == MEMORY_ROBOT)
    signs = robot_joint_signs;
  else
    signs = spark_joint_signs;

  // NOTE: we aren't doing anything right away, since sometimes what we want to do is bad
  float time_passed = frame_info_->seconds_since_start - time_motion_started_;
  if (time_passed < 1.0) {
    raw_joint_commands_->send_body_angles_ = false;
    raw_joint_commands_->send_arm_angles_ = false;
    raw_joint_commands_->send_head_pitch_angle_ = false;
    raw_joint_commands_->send_head_yaw_angle_ = false;
    raw_joint_commands_->send_stiffness_ = false;
    raw_joint_commands_->send_back_standup_ = false;
    raw_joint_commands_->send_sonar_command_ = false;
    return;
  }

  raw_joint_commands_->body_angle_time_ = processed_joint_commands_->body_angle_time_;
  raw_joint_commands_->arm_command_time_ = processed_joint_commands_->arm_command_time_;
  raw_joint_commands_->head_pitch_angle_time_ = processed_joint_commands_->head_pitch_angle_time_;
  raw_joint_commands_->head_yaw_angle_time_ = processed_joint_commands_->head_yaw_angle_time_;

  raw_joint_commands_->stiffness_time_ = processed_joint_commands_->stiffness_time_;
  raw_joint_commands_->send_back_standup_ = processed_joint_commands_->send_back_standup_;
/*
  // try to make the head hit its destination
  static float prevHeadTiltCommand = 0;
  static float prevHeadTilt = 0;
  bool headTiltCommandChanged = (fabs(processed_joint_commands_->angles_[HeadTilt] - prevHeadTiltCommand) > 0.01);
  bool headTiltChanged = (fabs(processed_joint_angles_->values_[HeadTilt] - prevHeadTilt) > DEG_T_RAD * 0.5);
  prevHeadTiltCommand = processed_joint_commands_->angles_[HeadTilt];
  prevHeadTilt = processed_joint_angles_->values_[HeadTilt];
  if ((processed_joint_angles_->stiffness_[HeadTilt] < 1e-5) || headTiltCommandChanged || headTiltChanged) {
    headTiltPID.reset();
  } else {
    processed_joint_commands_->angles_[HeadTilt] += headTiltPID.update(processed_joint_angles_->values_[HeadTilt],processed_joint_commands_->angles_[HeadTilt]);
    //std::cout << RAD_T_DEG * processed_joint_angles_->values_[HeadTilt] << std::endl;
    headTiltPID.cropCumulative(DEG_T_RAD * 2.0);
  }
*/
  // apply signs to joint commands
  for (int i=0; i<NUM_JOINTS; i++) {
    raw_joint_commands_->angles_[i] = signs[i] * processed_joint_commands_->angles_[i];
  }

  // handle head offsets for commands
  if (!processed_joint_commands_->head_pitch_angle_change_)
    raw_joint_commands_->angles_[HeadTilt] -= signs[HeadTilt] * robot_info_->dimensions_.values_[RobotDimensions::headTiltOffset];
  if (!processed_joint_commands_->head_yaw_angle_change_)
    raw_joint_commands_->angles_[HeadYaw] -= signs[HeadYaw] * robot_info_->dimensions_.values_[RobotDimensions::headPanOffset];


  // handle stiffness
  if (processed_joint_commands_->send_stiffness_) {
    for (int i=0; i<NUM_JOINTS; i++) {
      raw_joint_commands_->stiffness_[i] = processed_joint_commands_->stiffness_[i];
    }
  }
  raw_joint_commands_->send_stiffness_ = processed_joint_commands_->send_stiffness_;

  raw_joint_commands_->send_body_angles_ = processed_joint_commands_->send_body_angles_;
  raw_joint_commands_->send_arm_angles_ = processed_joint_commands_->send_arm_angles_;
  raw_joint_commands_->send_head_pitch_angle_ = processed_joint_commands_->send_head_pitch_angle_;
  raw_joint_commands_->send_head_yaw_angle_ = processed_joint_commands_->send_head_yaw_angle_;

  raw_joint_commands_->head_pitch_angle_change_ = processed_joint_commands_->head_pitch_angle_change_;
  raw_joint_commands_->head_yaw_angle_change_ = processed_joint_commands_->head_yaw_angle_change_;

  raw_joint_commands_->send_sonar_command_ = processed_joint_commands_->send_sonar_command_;
  raw_joint_commands_->sonar_command_ = processed_joint_commands_->sonar_command_;
}

void MotionCore::publishData() {
  memory_.motion_vision_lock_->lock();

  *sync_body_model_ = *body_model_;
  *sync_joint_angles_ = *processed_joint_angles_;
  *sync_sensors_ = *processed_sensors_;
  *sync_odometry_ = *odometry_;
  sync_kick_request_->kick_running_ = kick_request_->kick_running_;
  sync_kick_request_->finished_with_step_ = kick_request_->finished_with_step_;
  
  Pose2D temp_walk_target = sync_walk_request_->target_point_;
  bool temp_kick_with_left = sync_walk_request_->kick_with_left_;
  double temp_heading = sync_walk_request_->kick_heading_;
  WalkRequestBlock::Motion temp_motion = sync_walk_request_->motion_;
  *sync_walk_request_ = *walk_request_; // overwrite all
  sync_walk_request_->kick_with_left_ = temp_kick_with_left; // motion should never set foot for behavior
  sync_walk_request_->target_point_ = temp_walk_target;
  sync_walk_request_->kick_heading_ = temp_heading;
  sync_walk_request_->motion_ = temp_motion;
  *sync_walk_response_ = *walk_response_;
  sync_walk_request_->new_command_ = false;
  sync_walk_request_->slow_stand_ = false;
  sync_walk_request_->set_kick_step_params_ = false;

  *sync_processed_sonar_ = *processed_sonar_;
  *sync_walk_info_ = *walk_info_;

  memory_.motion_vision_lock_->unlock();
}

void MotionCore::receiveData() {
  memory_.motion_vision_lock_->lock();

  bool typechange = false;
  if(walk_request_->walk_type_ != sync_walk_request_->walk_type_)
    typechange = true;

  *kick_request_ = *sync_kick_request_;
  WalkControl temp_status = walk_request_->walk_control_status_;
  *walk_request_ = *sync_walk_request_;
  if (temp_status == WALK_CONTROL_DONE && sync_walk_request_->walk_control_status_ == WALK_CONTROL_SET)
    walk_request_->walk_control_status_ = temp_status;
  *processed_joint_commands_ = *sync_joint_commands_;
  *odometry_ = *sync_odometry_;

  if (sync_kick_params_->send_params_) {
    *kick_params_ = *sync_kick_params_;
    sync_kick_params_->send_params_ = false;
  }
  if (sync_walk_param_->send_params_) {
    *walk_param_ = *sync_walk_param_;
    sync_walk_param_->send_params_ = false;
  }
  if (sync_al_walk_param_->send_params_){
    *al_walk_param_ = *sync_al_walk_param_;
    sync_al_walk_param_->send_params_ = false;
  }
  
  if(typechange) {
    initWalkEngine();
  }

  memory_.motion_vision_lock_->unlock();
}


