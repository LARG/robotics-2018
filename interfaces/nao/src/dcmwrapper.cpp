#include <iostream>

#include <alvalue/alvalue.h>
#include <alcommon/alproxy.h>
#include <alproxies/dcmproxy.h>


#include <common/RobotInfo.h>
#include <common/InterfaceInfo.h>
#include <memory/LEDBlock.h>

#include "dcmwrapper.h"

const std::string DCMWrapper::body_position_name_ = "bodyActuator";
const std::string DCMWrapper::head_pitch_name_ = "headPitch";
const std::string DCMWrapper::head_yaw_name_ = "headYaw";
const std::string DCMWrapper::stiffness_name_ = "jointStiffness";

DCMWrapper::DCMWrapper() : initialized_(false) {
}

DCMWrapper::~DCMWrapper() {
}

void DCMWrapper::setProxy(boost::shared_ptr<AL::DCMProxy> pr) {
  dcm_proxy_ = pr;
}

void DCMWrapper::init() {
  std::cout << "DCMWrapper::Initializing\n" << std::flush;

  initModes();
  initAliases();
  initHands();
}

void DCMWrapper::initHands() {
  std::string keys[] = {"Device/SubDeviceList/LHand/Hardness/Actuator/Value","Device/SubDeviceList/RHand/Hardness/Actuator/Value","Device/SubDeviceList/LHand/Position/Actuator/Value","Device/SubDeviceList/RHand/Position/Actuator/Value"};
  float values[] = {1.0,1.0,0,0};
  for (int i = 0; i < 4; i++) {
    AL::ALValue commands;
    commands.arraySetSize(3);
    commands[0] = keys[i];
    commands[1] = std::string("Merge");
    commands[2].arraySetSize(1);
    commands[2][0].arraySetSize(2);
    commands[2][0][0] = values[i];
    commands[2][0][1] = dcm_proxy_->getTime(0);
    dcm_proxy_->set(commands);
  }
}


void DCMWrapper::initModes() {
  std::cout << "DCMWrapper::Creating DCM Modes list \n" << std::flush;
  dcm_modes_.clear();
  dcm_modes_.push_back(std::string("Merge"));
  dcm_modes_.push_back(std::string("ClearAll"));
  dcm_modes_.push_back(std::string("ClearAfter"));
  dcm_modes_.push_back(std::string("ClearBefore"));
}

void DCMWrapper::initAlias(const std::string &name, const std::string &deviceSuffix, int jointIndStart, int numJoints) {
  AL::ALValue result;
  AL::ALValue alias;
  alias.arraySetSize(2);
  alias[0] = std::string(name); // Alias for all body actuators
  alias[1].arraySetSize(numJoints);
  
  // Joints actuator list
  for (int i = 0; i < numJoints; i++)
    alias[1][i] = std::string("Device/SubDeviceList/") + getJointName((Joint)(i + jointIndStart)) + deviceSuffix;
  
  // Create alias
  try {
    result = dcm_proxy_->createAlias(alias);
  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "initDCMAlias()", "Error with DCM createAlias: " + e.toString());
  }
}

void DCMWrapper::initAliases() {
  std::cout << "DCMWrapper::Creating Aliases for Actuators \n" << std::flush;
  initAlias(body_position_name_,"/Position/Actuator/Value",BODY_JOINT_OFFSET,NUM_BODY_JOINTS);
  initAlias(head_pitch_name_,"/Position/Actuator/Value",HeadPitch,1);
  initAlias(head_yaw_name_,"/Position/Actuator/Value",HeadYaw,1);
  initAlias(stiffness_name_,"/Hardness/Actuator/Value",0,NUM_JOINTS);
  
  AL::ALValue result;
  AL::ALValue alias;
  // Alias for sonar
  alias.clear();
  alias.arraySetSize(2);
  alias[0] = std::string("usRequest");
  alias[1].arraySetSize(1);
  alias[1][0] = std::string("Device/SubDeviceList/US/Actuator/Value");

  // Create alias
  try {
    result = dcm_proxy_->createAlias(alias);
  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "initDCMAlias()", "Error with DCM createAlias for sonar: " + e.toString());
  }
  
  // Alias for leds
  alias.clear();
  alias.arraySetSize(2);
  alias[0] = std::string("ledActuators");
  alias[1].arraySetSize(NUM_LEDS);
 
  // led list
  for (int i = 0; i < NUM_LEDS; i++)
    alias[1][i] = std::string(getLEDString((LED)i));
  
  // Create alias
  try {
    result = dcm_proxy_->createAlias(alias);
  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "initDCMAlias()", "Error with DCM createAlias: " + e.toString());
  }
  
  // Now prepare the commands ALValue to store all values
  initCommands(body_position_commands_,body_position_name_,NUM_BODY_JOINTS);
  initCommands(head_pitch_commands_,head_pitch_name_,1);
  initCommands(head_yaw_commands_,head_yaw_name_,1);
  initCommands(stiffness_commands_,"jointStiffness",NUM_JOINTS);
  initSonarCommands();
  initLEDCommands();
}

void DCMWrapper::initSonar() {

  // Test different sonar modes instead of just initalizing sonar for regular use
  // testSonar();
  // return;

  // Send the complex Sonar command
  // sonar_commands_[4].arraySetSize(2);
  // sonar_commands_[5][0].arraySetSize(2);

  sonar_commands_[4][0] = dcm_proxy_->getTime(0);
  sonar_commands_[5][0][0] = 76.0f; //was 32.0f; 76 makes both sonars on all the time; see sonar manual for details
  // sonar_commands_[4][1] = dcm_proxy_->getTime(5000);
  // sonar_commands_[5][0][1] = 64.0f + 8.0f + 4.0f;
  dcm_proxy_->setAlias(sonar_commands_);

  // sonar_commands_[4].arraySetSize(1);
  // sonar_commands_[5][0].arraySetSize(1);
}

void DCMWrapper::testSonar() {

  // Send the complex Sonar command
  sonar_commands_[4].arraySetSize(13);
  sonar_commands_[5][0].arraySetSize(13);

  int time = dcm_proxy_->getTime(0);
  int inc = 10000;
  time+=inc;

  sonar_commands_[4][0] = time;
  sonar_commands_[5][0][0] = 32.0f;

  time += inc;
  sonar_commands_[4][1] = time;
  sonar_commands_[5][0][1] = 64.0f + 12.0f;
  time += inc;
  sonar_commands_[4][2] = time;
  sonar_commands_[5][0][2] = 32.0f;

  time += inc;
  sonar_commands_[4][3] = time;
  sonar_commands_[5][0][3] = 64.0f + 12.0f;
  time += inc;
  sonar_commands_[4][4] = time;
  sonar_commands_[5][0][4] = 32.0f;

  time += inc;
  sonar_commands_[4][5] = time;
  sonar_commands_[5][0][5] = 64.0f + 1.0f;
  time += inc;
  sonar_commands_[4][6] = time;
  sonar_commands_[5][0][6] = 32.0f;

  time += inc;
  sonar_commands_[4][7] = time;
  sonar_commands_[5][0][7] = 64.0f + 2.0f;
  time += inc;
  sonar_commands_[4][8] = time;
  sonar_commands_[5][0][8] = 32.0f;

  time += inc;
  sonar_commands_[4][9] = time;
  sonar_commands_[5][0][9] = 64.0f + 3.0f;
  time += inc;
  sonar_commands_[4][10] = time;
  sonar_commands_[5][0][10] = 32.0f;

  time += inc;
  sonar_commands_[4][11] = time;
  sonar_commands_[5][0][11] = 64.0f + 4.0f;
  time += inc;
  sonar_commands_[4][12] = time;
  sonar_commands_[5][0][12] = 32.0f;
  dcm_proxy_->setAlias(sonar_commands_);

  sonar_commands_[4].arraySetSize(1);
  sonar_commands_[5][0].arraySetSize(1);
}

void DCMWrapper::initSonarCommands() {
  sonar_commands_.arraySetSize(6);
  sonar_commands_[0] = std::string("usRequest");
  sonar_commands_[1] = std::string("Merge"); // BHuman says that ClearAll doesn't work
  sonar_commands_[2] = std::string("time-separate");
  sonar_commands_[3] = 0;
  sonar_commands_[4].arraySetSize(1);
  sonar_commands_[5].arraySetSize(1);
  sonar_commands_[5][0].arraySetSize(1);
}


void DCMWrapper::initLEDCommands() {
  led_commands_.arraySetSize(6);
  led_commands_[0] = std::string("ledActuators");
  led_commands_[1] = std::string("ClearAll");
  led_commands_[2] = std::string("time-separate");
  led_commands_[3] = 0;
  
  led_commands_[4].arraySetSize(1);
  led_commands_[5].arraySetSize(NUM_LEDS);
  for (int32_t i=0; i<NUM_LEDS; i++) {
    led_commands_[5][i].arraySetSize(1);
  }
}

void DCMWrapper::initCommands(AL::ALValue &commands, const std::string &commandType, int numJoints)
{
	// Prepare commands for joint stuff
	commands.arraySetSize(6);
	commands[0] = commandType;
  commands[1] = std::string("ClearAll"); // Erase all previous commands
	commands[2] = std::string("time-separate");
	commands[3] = 0;

	commands[4].arraySetSize(1);
  commands[4][0] = dcm_proxy_->getTime(1000000); // This is a bad command so set it to be far in the future
	//commands[4][0]  Will be the new time

	commands[5].arraySetSize(numJoints); // For all joints

	for (int32_t i = 0; i < numJoints; i++) {
			commands[5][i].arraySetSize(1);
      commands[5][i][0] = 0.0f;
			//commands[5][i][0] will be the new value
	}
}

void DCMWrapper::sendJointCommands(bool send, AL::ALValue &commands, float time, float angles[NUM_JOINTS], int jointIndStart, int numJoints) {
  if(send) {
    commands[4][0] = dcm_proxy_->getTime(time);
    for (int32_t i = 0; i < numJoints; i++) {

      commands[5][i][0] = angles[i + jointIndStart];
    }
    dcm_proxy_->setAlias(commands);
  }
}

//Send the actual commands to the actuators
void DCMWrapper::sendToActuators(JointCommandBlock *raw_joint_commands, JointBlock *raw_joint_angles_) {
  //  Note : a joint could be ignore unsing a NAN value.
  //  Note : do not forget to set stiffness
  if (USE_AL_MOTION)
    return;

  try {
    sendJointCommands(raw_joint_commands->send_body_angles_,body_position_commands_,raw_joint_commands->body_angle_time_,raw_joint_commands->angles_.data(),BODY_JOINT_OFFSET,NUM_BODY_JOINTS);

    //cout << "send body angle bool: " << raw_joint_commands->send_body_angles_ << endl; //RSWalk test
//    cout << "LHipPitch receives command: " << raw_joint_commands->angles_[4] << endl; 
//    cout << "LKneePitch receives command: " << raw_joint_commands->angles_[5] << endl; 
//    cout << "LAnklePitch receives command: " << raw_joint_commands->angles_[6] << endl; 

    // head pitch is a change
    if (raw_joint_commands->send_head_pitch_angle_ && raw_joint_commands->head_pitch_angle_change_){
      raw_joint_commands->angles_[HeadPitch] += raw_joint_angles_->values_[HeadPitch];
    }

    // head yaw is a change
    if (raw_joint_commands->send_head_yaw_angle_ && raw_joint_commands->head_yaw_angle_change_){
      raw_joint_commands->angles_[HeadYaw] += raw_joint_angles_->values_[HeadYaw];
    }

    sendJointCommands(raw_joint_commands->send_head_pitch_angle_,head_pitch_commands_,raw_joint_commands->head_pitch_angle_time_,raw_joint_commands->angles_.data(),HeadPitch,1);
    sendJointCommands(raw_joint_commands->send_head_yaw_angle_,head_yaw_commands_,raw_joint_commands->head_yaw_angle_time_,raw_joint_commands->angles_.data(),HeadYaw,1);
    
    sendJointCommands(raw_joint_commands->send_stiffness_,stiffness_commands_,raw_joint_commands->stiffness_time_,raw_joint_commands->stiffness_.data(),0,NUM_JOINTS);
    
    // optionally send sonar commands: this part is removed; both sonars would be transmitting and receving all the time
//    if (true) { //(raw_joint_commands->send_sonar_command_) {
//      sonar_commands_[4][0] = dcm_proxy_->getTime(0); // immediately
//      sonar_commands_[5][0][0] = 76.0f; //raw_joint_commands->sonar_command_;
//      dcm_proxy_->setAlias(sonar_commands_);
//    }

  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "sendToActuators()", "Error sending some commands : " + e.toString());
  }
}

//Send the actual commands to the actuators
void DCMWrapper::sendToLEDs(LEDBlock *led_block) {

  if (!led_block->send_leds_) return;
  led_block->send_leds_=false; // reset variable
  
  //  Note : a joint could be ignore unsing a NAN value.
  //  Note : do not forget to set stiffness
  int32_t i;
  try {
    // Set new time
    led_commands_[4][0] =  dcm_proxy_->getTime(0);
    // Set new led commands
    for (i=0; i<NUM_LEDS; i++) {
      led_commands_[5][i][0] = led_block->values_[i];
    }
    // send them
    dcm_proxy_->setAlias(led_commands_);
  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "sendToLEDs()", "Error sending some commands : " + e.toString());
  }
}


//void DCMWrapper::sendLED() {
//}
/*
void DCMWrapper::frontGetup(){
  try {
   AL::ALValue commandsAlias;
   AL::ALValue commands;
   
   commandsAlias.arraySetSize(2);
   commandsAlias[0] = std::string("ChestLeds");
   commandsAlias[1].arraySetSize(3);
   commandsAlias[1][0] = std::string("ChestBoard/Led/Red/Actuator/Value");
   commandsAlias[1][1] = std::string("ChestBoard/Led/Green/Actuator/Value");
   commandsAlias[1][2] = std::string("ChestBoard/Led/Blue/Actuator/Value");
   
   dcm_proxy_->createAlias(commandsAlias);
   
   commands.arraySetSize(6);
   commands[0] = std::string("ChestLeds");
   commands[1] = std::string("ClearAll");
   commands[2] = std::string("time-separate");
   commands[3] = 0;
   
   commands[4].arraySetSize(6);
   commands[4][0] = dcm_proxy_->getTime(10000);
   commands[4][1] = dcm_proxy_->getTime(20000);
   commands[4][2] = dcm_proxy_->getTime(30000);
   commands[4][3] = dcm_proxy_->getTime(40000);
   commands[4][4] = dcm_proxy_->getTime(50000);
   commands[4][5] = dcm_proxy_->getTime(60000);
   
   commands[5].arraySetSize(3);
   
   // ChestBoard/Led/Red/Actuator/Value
   commands[5][0].arraySetSize(6);
   commands[5][0][0] = 1.0;
   commands[5][0][1] = 0.0;
   commands[5][0][2] = 1.0;
   commands[5][0][3] = 0.0;
   commands[5][0][4] = 1.0;
   commands[5][0][5] = 0.0;
   
   // ChestBoard/Led/Green/Actuator/Value
   commands[5][1].arraySetSize(6);
   commands[5][1][0] = 1.0;
   commands[5][1][1] = 0.5;
   commands[5][1][2] = 1.0;
   commands[5][1][3] = 0.25;
   commands[5][1][4] = 0.125;
   commands[5][1][5] = 0.0; 
   
   // ChestBoard/Led/Blue/Actuator/Value
   commands[5][2].arraySetSize(6);
   commands[5][2][0] = 0.0625;
   commands[5][2][1] = 0.125;
   commands[5][2][2] = 0.25;
   commands[5][2][3] = 0.50;
   commands[5][2][4] = 0.75;
   commands[5][2][5] = 1.0;
   
   dcm_proxy_->setAlias(commands);
  } catch (const AL::ALError &e) {
    throw ALERROR("naointerface", "backGetup()", "Error sending some commands : " + e.toString());
  }
}
*/
