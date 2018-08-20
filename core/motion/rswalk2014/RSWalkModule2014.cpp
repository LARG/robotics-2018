#include "RSWalkModule2014.h"
#include "WalkEnginePreProcessor.hpp"
#include "Walk2014Generator.hpp"
#include "DistributedGenerator.hpp"
#include "ClippedGenerator.hpp"

#include <math/Geometry.h>

// Runswift files
#include "types/JointValues.hpp"
#include "types/AbsCoord.hpp"
#include "perception/kinematics/Kinematics.hpp"
#include "BodyModel.hpp"

#include <memory/BodyModelBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/KickRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/RobotInfoBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WalkInfoBlock.h>
#include <memory/WalkParamBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/SpeechBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/RobotStateBlock.h>

#include <common/Kicks.h>

#include "GyroConfig.h"

#define ODOMETRY_LAG 8

#define DEBUG_OUTPUT false
#define DEBUG_PRINT(arg1, arg2, arg3, arg4) if (DEBUG_OUTPUT) std::cout << arg1 << arg2 << arg3 << arg4 << std::endl;


/*-----------------------------------------------------------------------------
* Motion thread tick function (copy from runswift MotionAdapter.cpp)
This would be like processFrame() function as in our code
*---------------------------------------------------------------------------*/
void RSWalkModule2014::processFrame() {
  
  // If we are changing commands we need to reset generators
  if(walk_request_->motion_ != prev_command_ && !walk_request_->perform_kick_ and
     (not isRequestForWalk(walk_request_->motion_) or not isRequestForWalk(prev_command_))) {
    DEBUG_PRINT("RESETTING GENERATORS! ",
                WalkRequestBlock::getName(walk_request_->motion_),
                " != ", WalkRequestBlock::getName(prev_command_))
    standing = true;
  }


  // 1. Need odometry to send to WalkGenerator. makeJoints updates odometry for localization
  Odometry odo = Odometry(odometry_->displacement.translation.x,
                          odometry_->displacement.translation.y,
                          odometry_->displacement.rotation);
  Odometry prev = Odometry(odo);  // Copy for after makeJoints call

  // 1.1 Get ball position relative to robot - this will be changed to a
  // target position for line-up
  float ballX = 0;
  float ballY = 0;
  DEBUG_PRINT("BallX: ", ballX, " BallY: ", ballY);

  // 2. Convert our request to runswift request
  ActionCommand::Head head;  // Use default head. Not using HeadGenerator
  ActionCommand::Body body;
  body.actionType = ActionCommand::Body::WALK;
  body.forward = 0; body.left = 0; body.turn=0;
  body.power = 1;
  body.bend = 1;

  DEBUG_PRINT("Requested: ", WalkRequestBlock::getName(walk_request_->motion_), "", "");
  if (walk_request_->motion_ == WalkRequestBlock::STAND) {
    // Do nothing for bent-leg stand
  } else if (walk_request_->motion_ == WalkRequestBlock::STAND_STRAIGHT) {
    body.power = 0.4;
    body.bend = 0;
  } else if (walk_request_->motion_ == WalkRequestBlock::WALK) {
    body.forward = walk_request_->speed_.translation.x;
    body.left = walk_request_->speed_.translation.y;
    body.turn = walk_request_->speed_.rotation;

    if (walk_params_->use_sprint_params_)
      body.speed = walk_params_->sprint_params_.speed;
    else
      body.speed = walk_params_->main_params_.speed;

  } else if (walk_request_->motion_ == WalkRequestBlock::NONE or
             walk_request_->motion_ == WalkRequestBlock::WAIT) {
    body.actionType = ActionCommand::Body::NONE;
  } else {
    // Default to tall straight stand
    body.bend = 0;
  }

  if (not body_model_->feet_on_ground_ and
    body.actionType != ActionCommand::Body::NONE) {
    // Need something here so robot stops when picked up
    // body.actionType = ActionCommand::Body::REF_PICKUP;
    toStandRequest(body, false);
  }

  // 2.3 LEDs and Sonar -- Not important, use defaults
  ActionCommand::LED leds;
  float sonar = 0.0;

  // Now we have everything we need for a runswift request
  ActionCommand::All request(head, body, leds, sonar);

  // 3. Create runswift sensor object
  SensorValues sensors;

  // 3.1 Sense Joints 
  for (int ut_ind = 0; ut_ind < NUM_JOINTS; ut_ind++){
    if (ut_ind != RHipYawPitch) { // RS does not have this joint because RHYP is same as LHYP on Nao
      int rs_ind = utJointToRSJoint[ut_ind];
      sensors.joints.angles[rs_ind] = joints_->values_[ut_ind] * robot_joint_signs[ut_ind]; // changed from raw_joints - Josiah
      sensors.joints.temperatures[rs_ind] = sensors_->joint_temperatures_[ut_ind];
    }
  }

  // Detect if legs are getting hot
  if(sensors_->joint_temperatures_[RKneePitch] > 100 ||
     sensors_->joint_temperatures_[RHipPitch] > 100)
    bodyModel.isRightLegHot = true;
  if(sensors_->joint_temperatures_[LKneePitch] > 100 ||
     sensors_->joint_temperatures_[LHipPitch] > 100)
    bodyModel.isLeftLegHot = true;

  // 3.2 Sensors
  for (int ut_ind = 0; ut_ind < bumperRR + 1; ut_ind++){
    int rs_ind = utSensorToRSSensor[ut_ind];
    sensors.sensors[rs_ind] = sensors_->values_[ut_ind];
  }

  // 4. If robot is remaining still, calbrate x and y gyroscopes
  bool not_calibrated = updateGyroScopeCalibration(sensors.sensors[RSSensors::InertialSensor_GyrX],
                                                   sensors.sensors[RSSensors::InertialSensor_GyrY]);
  odometry_->walkDisabled = not_calibrated;
  // After each gyro has completed two cycles it is calibrated.
  if (calX_count >= 2)
    bodyModel.isGyroXCalibrated = true;
  if (calY_count >= 2)
    bodyModel.isGyroYCalibrated = true;

  // Apply offset and convert from rad/sec to rad /frame
  sensors.sensors[RSSensors::InertialSensor_GyrX] = (sensors.sensors[RSSensors::InertialSensor_GyrX] - offsetX) * 0.01;
  sensors.sensors[RSSensors::InertialSensor_GyrY] = (sensors.sensors[RSSensors::InertialSensor_GyrY] - offsetY) * 0.01;

  // 5. Prepare BodyModel to pass to makeJoints
  // 5.1 Get Kinematics ready for bodyModel. We should figure out what parameter values should be.
  kinematics.setSensorValues(sensors); 
  kinematics.updateDHChain();

  // Resetting generators and moving to intermediate stance
  if (standing) {

    if (standing) {
      clipper->reset();
      DEBUG_PRINT("Resetting clipper", "", "", "");
      request.body = ActionCommand::Body::INITIAL;
      odo.clear();
    }
  }

  // 5.2 update bodyModel
  bodyModel.kinematics = &kinematics;

  // If a walk is requested but we are not calibrated then complain loudly.
  if ((request.body.forward != 0 or
       request.body.left != 0 or
       request.body.turn != 0) and odometry_->walkDisabled) {
    static int delay_ct = 0;
    if (delay_ct % 100 == 0)
      speech_->say("Not Calibrated");
      delay_ct ++;
    DEBUG_PRINT("Not calibrated", "", "", "");
    if (odometry_->standing) {
      toStandRequest(request.body, true);
    }
    else
      request.body = ActionCommand::Body::NONE;
  }

  DEBUG_PRINT("motion request: ",
              WalkRequestBlock::getName(walk_request_->motion_), ", prev: ",
              WalkRequestBlock::getName(prev_command_));
  DEBUG_PRINT("Body Request: ", static_cast<int>(request.body.actionType), "", "");
  prev_command_ = walk_request_->motion_;

  bool requestWalkKick = bodyModel.walkKick;
  // Call the clipped generator which calls the distributed generator to produce walks, stands, etc.
  JointValues joints = clipper->makeJoints(&request, &odo, sensors, bodyModel, ballX, ballY);

  // Update odometry
  static double cum_f = 0, cum_l = 0, cum_t = 0;
  odometry_->displacement.translation.x = odo.forward;
  odometry_->displacement.translation.y = odo.left;
  odometry_->displacement.rotation = odo.turn;
  odometry_->standing = clipper->isStanding();
  Odometry delta = Odometry(odo - prev);
  cum_f += delta.forward;
  cum_l += delta.left;
  cum_t += delta.turn;

  // Update walk_info_
  walk_info_->instability_ = avg_gyroX;
  if (avg_gyroX < -2 or avg_gyroX > 2) {
    walk_info_->instable_ = true;
  } else {
    walk_info_->instable_ = false;
  }

  wasKicking = kick_request_->kick_running_;

  if (request.body.actionType == ActionCommand::Body::NONE) {
    return;
  }

  // Convert RS joints to UT joints and write to commands memory block
  for (int ut_ind = BODY_JOINT_OFFSET; ut_ind < NUM_JOINTS; ut_ind++) {
    if (ut_ind != RHipYawPitch) { // RS does not have this joint
      int rs_ind = utJointToRSJoint[ut_ind];
      commands_->angles_[ut_ind] = robot_joint_signs[ut_ind] * joints.angles[rs_ind];
      commands_->stiffness_[ut_ind] = joints.stiffnesses[rs_ind];
    } 
  }

  commands_->stiffness_[RHipYawPitch] = commands_->stiffness_[LHipYawPitch];
  commands_->angles_[RHipYawPitch] = commands_->angles_[LHipYawPitch];

  // Walk keeps falling backwards when knees over 100. Leaning forward helps some
  if (request.body.actionType == ActionCommand::Body::STAND and
      (bodyModel.isLeftLegHot || bodyModel.isRightLegHot)) {
    commands_->angles_[RHipPitch] -= DEG_T_RAD * 2;
    commands_->angles_[LHipPitch] -= DEG_T_RAD * 2;
  }
  
  // if the robot has walked, listen to arm command from WalkGenerator, else do stiffness
  if (request.body.actionType != ActionCommand::Body::WALK) {
    for (int ind = ARM_JOINT_FIRST; ind <= ARM_JOINT_LAST; ind ++)
      commands_->stiffness_[ind] = 1.0;
  }

  commands_->send_body_angles_ = true;
  commands_->send_stiffness_ = true;
  commands_->body_angle_time_ = 10;
  commands_->stiffness_time_ = 10;
  standing = false;

}


void RSWalkModule2014::readOptions(std::string path)
{

  clipper->readOptions(path);

}

RSWalkModule2014::RSWalkModule2014():
    step_into_kick_state_(NONE),
    time_step_into_kick_finished_(0),
    last_gyroY_time(-1),
    last_gyroX_time(-1)
{
    utJointToRSJoint[HeadYaw] = RSJoints::HeadYaw;
    utJointToRSJoint[HeadPitch] = RSJoints::HeadPitch;

    utJointToRSJoint[LShoulderPitch] = RSJoints::LShoulderPitch;
    utJointToRSJoint[LShoulderRoll] = RSJoints::LShoulderRoll;
    utJointToRSJoint[LElbowYaw] = RSJoints::LElbowYaw;
    utJointToRSJoint[LElbowRoll] = RSJoints::LElbowRoll;

    utJointToRSJoint[RShoulderPitch] = RSJoints::RShoulderPitch;
    utJointToRSJoint[RShoulderRoll] = RSJoints::RShoulderRoll;
    utJointToRSJoint[RElbowYaw] = RSJoints::RElbowYaw;
    utJointToRSJoint[RElbowRoll] = RSJoints::RElbowRoll;

    utJointToRSJoint[LHipYawPitch] = RSJoints::LHipYawPitch;
    utJointToRSJoint[LHipRoll] = RSJoints::LHipRoll;
    utJointToRSJoint[LHipPitch] = RSJoints::LHipPitch;
    utJointToRSJoint[LKneePitch] = RSJoints::LKneePitch;
    utJointToRSJoint[LAnklePitch] = RSJoints::LAnklePitch;
    utJointToRSJoint[LAnkleRoll] = RSJoints::LAnkleRoll;

    utJointToRSJoint[RHipYawPitch] = -1;//RSJoints::RHipYawPitch;
    utJointToRSJoint[RHipRoll] = RSJoints::RHipRoll;
    utJointToRSJoint[RHipPitch] = RSJoints::RHipPitch;
    utJointToRSJoint[RKneePitch] = RSJoints::RKneePitch;
    utJointToRSJoint[RAnklePitch] = RSJoints::RAnklePitch;
    utJointToRSJoint[RAnkleRoll] = RSJoints::RAnkleRoll;

    // SENSOR VALUE CONVERSION
    utSensorToRSSensor[gyroX] = RSSensors::InertialSensor_GyrX;
    utSensorToRSSensor[gyroY] = RSSensors::InertialSensor_GyrY;   
    utSensorToRSSensor[gyroZ] = RSSensors::InertialSensor_GyrRef;   
    utSensorToRSSensor[accelX] = RSSensors::InertialSensor_AccX;
    utSensorToRSSensor[accelY] = RSSensors::InertialSensor_AccY;
    utSensorToRSSensor[accelZ] = RSSensors::InertialSensor_AccZ;   
    utSensorToRSSensor[angleX] = RSSensors::InertialSensor_AngleX;
    utSensorToRSSensor[angleY] = RSSensors::InertialSensor_AngleY;
    utSensorToRSSensor[angleZ] = RSSensors::InertialSensor_AngleZ;
    utSensorToRSSensor[battery] = RSSensors::Battery_Current;
    utSensorToRSSensor[fsrLFL] = RSSensors::LFoot_FSR_FrontLeft;
    utSensorToRSSensor[fsrLFR] = RSSensors::LFoot_FSR_FrontRight;   
    utSensorToRSSensor[fsrLRL] = RSSensors::LFoot_FSR_RearLeft;
    utSensorToRSSensor[fsrLRR] = RSSensors::LFoot_FSR_RearRight;
    utSensorToRSSensor[fsrRFL] = RSSensors::RFoot_FSR_FrontLeft;   
    utSensorToRSSensor[fsrRFR] = RSSensors::RFoot_FSR_FrontRight;
    utSensorToRSSensor[fsrRRL] = RSSensors::RFoot_FSR_RearLeft;
    utSensorToRSSensor[fsrRRR] = RSSensors::RFoot_FSR_RearRight;   
    utSensorToRSSensor[centerButton] = RSSensors::ChestBoard_Button;   
    utSensorToRSSensor[bumperLL] = RSSensors::LFoot_Bumper_Left;
    utSensorToRSSensor[bumperLR] = RSSensors::LFoot_Bumper_Right;
    utSensorToRSSensor[bumperRL] = RSSensors::RFoot_Bumper_Left;   
    utSensorToRSSensor[bumperRR] = RSSensors::RFoot_Bumper_Right;
    // No UT Sensors for RSSensors RFoot_FSR_CenterOfPressure_X/Y
    // Battery_Charge or US, not sure what these are but Runswift has them.

}

RSWalkModule2014::~RSWalkModule2014() {
}


void RSWalkModule2014::specifyMemoryDependency() {
    requiresMemoryBlock("frame_info");
    requiresMemoryBlock("processed_joint_angles");
    requiresMemoryBlock("raw_joint_angles");
    requiresMemoryBlock("processed_joint_commands");
    requiresMemoryBlock("kick_request");
    requiresMemoryBlock("odometry");
    requiresMemoryBlock("robot_info");
    requiresMemoryBlock("raw_sensors");
    requiresMemoryBlock("walk_info");
    requiresMemoryBlock("walk_param");
    requiresMemoryBlock("walk_request");
    requiresMemoryBlock("body_model");
    requiresMemoryBlock("speech");
    requiresMemoryBlock("robot_state");
}

void RSWalkModule2014::specifyMemoryBlocks() {
    getOrAddMemoryBlock(frame_info_,"frame_info");
    getOrAddMemoryBlock(joints_,"processed_joint_angles");
    getOrAddMemoryBlock(raw_joints_,"raw_joint_angles");
    getOrAddMemoryBlock(commands_,"processed_joint_commands");
    getOrAddMemoryBlock(kick_request_,"kick_request");
    getOrAddMemoryBlock(odometry_,"odometry");
    getOrAddMemoryBlock(robot_info_,"robot_info");
    getOrAddMemoryBlock(sensors_,"raw_sensors");
    getOrAddMemoryBlock(walk_info_,"walk_info");
    getOrAddMemoryBlock(walk_params_,"walk_param");
    getOrAddMemoryBlock(walk_request_,"walk_request");
    getOrAddMemoryBlock(body_model_,"body_model");
    getOrAddMemoryBlock(speech_, "speech");
    memory_->getBlockByName(robot_state_, "robot_state", MemoryOwner::VISION);
    memory_->getOrAddBlockByName(speech_,"speech",MemoryOwner::SHARED);
}

void RSWalkModule2014::initSpecificModule() {
    // For RSWalk
    generator = new WalkEnginePreProcessor(); 
    std::string config_path = memory_->data_path_;
    config_path += "/config/rswalk2014"; 
    clipper = new ClippedGenerator((Generator*) new DistributedGenerator(config_path));
    // clipper->reset();
    readOptions(config_path);
    standing = false;
    prev_command_ = WalkRequestBlock::NONE;
    x_target = -1;
    y_target = -1;
    wasKicking = false;


    auto path = util::format("%s/%i_xy.yaml", util::cfgpath(util::GyroConfigs), robot_state_->robot_id_);
    GyroConfig config;
    if(config.load(path)) {
      printf("Loaded XY gyro configuration for Robot %02i.\n", robot_state_->robot_id_);
      offsetX = config.offsetX;
      offsetY = config.offsetY;
      calibration_write_time = config.calibration_write_time;
    }

}


void RSWalkModule2014::selectivelySendStiffness() {
    for (int i = 0; i < NUM_JOINTS; i++) {
        if (fabs(joints_->stiffness_[i] - commands_->stiffness_[i]) > 0.01) {
            commands_->send_stiffness_ = true;
            commands_->stiffness_time_ = 10;
            return;
        }
    }
}

void RSWalkModule2014::handleStepIntoKick() {}

bool RSWalkModule2014::updateGyroScopeCalibration(float cur_gyroX, float cur_gyroY) {

  // 1. Calibrate gyroX
  bool not_calibrated = true;
  double delta_gyroX = abs(cur_gyroX - last_gyroX);
  // maintain moving averages for gyro and delta_gyro
  avg_gyroX = avg_gyroX * (1.0 - window_size) + cur_gyroX * window_size;
  avg_delta_gyroX = avg_delta_gyroX * (1.0- window_size) + delta_gyroX * window_size;
  if (avg_delta_gyroX < delta_threshold) {
    // robot remains still, do calibration
    offsetX = avg_gyroX;
    // reset avg_delta so it does not keep recalibrating
    avg_delta_gyroX = reset;
    calX_count += 1; 
    if (calX_count == 1) {
      cout << "(First calibration, may not be accurate) A GyroX calibration was triggered, offsetX set to: " << offsetX << endl;
    } else {
      cout << "A GyroX calibration was triggered, offsetX set to: " << offsetX << endl;
    }
    last_gyroX_time = frame_info_-> seconds_since_start;
  }
  else {
    DEBUG_PRINT("avg_delta_gyroX is: ", avg_delta_gyroX, " gyroX not stable, no calibration", "");
  }

  last_gyroX = cur_gyroX;

  // 2. Calibrate gyroY
  double delta_gyroY = abs(cur_gyroY - last_gyroY);
  // maintain moving averages for gyro and delta_gyro
  avg_gyroY = avg_gyroY * (1.0 - window_size) + cur_gyroY * window_size;
  avg_delta_gyroY = avg_delta_gyroY * (1.0- window_size) + delta_gyroY * window_size;
  if (avg_delta_gyroY < delta_threshold) {
    // robot remains still, do calibration
    offsetY = avg_gyroY;
    // reset avg_delta so it does not keep recalibrating
    avg_delta_gyroY = reset;
    calY_count += 1;
    if (calY_count == 1) {
      cout << "(First calibration, may not be accurate) A GyroY calibration was triggered, offsetY set to: " << offsetY << endl;
    } else {     
      cout << "A GyroY calibration was triggered, offsetY set to: " << offsetY << endl;
    }
    last_gyroY_time = frame_info_-> seconds_since_start;
  } else {
    DEBUG_PRINT("avg_delta_gyroY is: ", avg_delta_gyroY, " gyroY not stable, no calibration", "");
  }

  last_gyroY = cur_gyroY;

  if ((calX_count >= 2 && calY_count >= 2) or (getSystemTime() - calibration_write_time < 600)) {
    not_calibrated = false;
    calX_count = 2;
    calY_count = 2;
    
    if (last_gyroY_time  > last_calibration_write + 30 and last_gyroX_time > last_calibration_write + 30) {
      calibration_write_time = getSystemTime();
      GyroConfig config;
      config.offsetX = offsetX;
      config.offsetY = offsetY;
      config.calibration_write_time = calibration_write_time;
            auto path = util::format("%s/%i_xy.yaml", util::cfgpath(util::GyroConfigs), robot_state_->robot_id_);
      config.save(path);
      last_calibration_write = frame_info_->seconds_since_start;
    }
  }

  return not_calibrated;
}

bool RSWalkModule2014::isRequestForWalk(WalkRequestBlock::Motion motion) {
  return (motion == WalkRequestBlock::WALK or
          motion == WalkRequestBlock::STAND or
          motion == WalkRequestBlock::STAND_STRAIGHT);
}

void RSWalkModule2014::setBodyForKick(ActionCommand::Body &body, float &ballX, float &ballY) {
  ballX = walk_request_->target_point_.translation.x;
  ballY = walk_request_->target_point_.translation.y;
  body.foot = (walk_request_->kick_with_left_ ? ActionCommand::Body::LEFT : ActionCommand::Body::RIGHT);
}

void RSWalkModule2014::toStandRequest(ActionCommand::Body &body, bool use_straight_stand) {
  body.actionType = ActionCommand::Body::WALK;
  body.forward = 0;
  body.left = 0;
  body.turn = 0;
  body.bend = 1;
  if (use_straight_stand)
    body.bend = 0;
}
