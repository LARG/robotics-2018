/**
 * @author Michael Quinlan
 *
 * Version : $Id$
 * This file was generated by Aldebaran Robotics ModuleGenerator
 */

#include "naointerface.h"

#include <alvalue/alvalue.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>

#include <almemoryfastaccess/almemoryfastaccess.h>
#include <alproxies/dcmproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/almotionproxy.h>

#include <alproxies/altexttospeechproxy.h>

#include <boost/lexical_cast.hpp>

#include <common/RobotInfo.h>
#include <common/InterfaceInfo.h>

#include <chrono>

MemoryFrame* naointerface::MEMORY_INSTANCE = NULL;

int parseVersion(std::string version) {
  float vf;
  sscanf(version.c_str(), "V%f", &vf);
  int v = vf * 10;
  return v;
}
 
/**
 * Constructor for naointerface object
 * @param broker The parent broker
 * @param name The name of the module
 */
naointerface::naointerface( boost::shared_ptr<AL::ALBroker> broker, const std::string& name):
  AL::ALModule(broker, name)
{
  std::cout << "NaoInterface::Starting Interface Module" << std::endl << std::flush;

  fast_sensor_access_ = boost::shared_ptr<AL::ALMemoryFastAccess>(new AL::ALMemoryFastAccess());
  dcmWrap_ = new DCMWrapper();
  if (USE_AL_MOTION)
    al_motion_wrap_ = new ALMotionWrapper();

  initMemory();
  // set up locks
  cleanLock(Lock::getLockName(memory_,LOCK_MOTION));
  motion_lock_ = new Lock(Lock::getLockName(memory_,LOCK_MOTION));

  setModuleDescription("Interface between the robot and the Austin Villa Code base");
  start();

  std::string body_id = al_memory_->getData("Device/DeviceList/ChestBoard/BodyId");
  body_id = body_id.substr(0, body_id.length() - 1);
  std::copy(body_id.begin(), body_id.end() + 1, robot_state_->body_id_.data());
  std::string head_version = al_memory_->getData("RobotConfig/Head/BaseVersion");
  robot_state_->head_version_ = parseVersion(head_version);
  std::string body_version = al_memory_->getData("RobotConfig/Body/BaseVersion");
  robot_state_->body_version_ = parseVersion(body_version);
  printf("body id: '%s' --> '%s'\n", body_id.c_str(), robot_state_->body_id_.data());
  printf("head version: '%s' --> %i\n", head_version.c_str(), robot_state_->head_version_);
  printf("body version: '%s' --> %i\n", body_version.c_str(), robot_state_->body_version_);
}

/**
 * Destructor for naointerface object
 */
naointerface::~naointerface() {}

void naointerface::start() {
  // Create the proxy to dcm
  try {
    std::cout << "NaoInterface::Creating DCM Proxy\n" << std::flush;
    // Get the DCM proxy
    //dcm_proxy_ = getParentBroker()->getDcmProxy();
    dcmWrap_->setProxy(getParentBroker()->getDcmProxy());
    al_memory_ = getParentBroker()->getMemoryProxy();
  } catch (AL::ALError& e) {
    throw ALERROR(getName(), "start()", "Impossible to create DCM Proxy : " + e.toString());
  }
  
  if (USE_AL_MOTION) {
    try {
      std::cout << "NaoInterface::Creating ALMotion Proxy\n" << std::flush;
      std::cerr << "ALMOTION UNSUPPORTED" << std::endl;
      ::exit(42);
      // Get the ALMotion proxy
      //al_motion_wrap_->setProxy(getParentBroker()->getMotionProxy());
      std::cout << "DONE" << std::flush << std::endl;
    } catch (AL::ALError& e) {
      throw ALERROR(getName(), "start()", "Impossible to create ALMotion Proxy : " + e.toString());
    }
  }

  try {
    std::cout << "NaoInterface::Creating Text to Speech Proxy\n" << std::flush;
    tts_proxy_ = boost::shared_ptr<AL::ALTextToSpeechProxy>(new AL::ALTextToSpeechProxy(getParentBroker()));
    tts_proxy_->setVolume(1.0);
    std::cout << "DONE" << std::flush << std::endl;
  } catch (AL::ALError& e) {
    throw ALERROR(getName(), "start()", "Impossible to create TextToSpeech Proxy : " + e.toString());
  }



  //init audio interface
  /*try {
	
   
    std::cout << "NaoInterface::Creating ALSoundBasedReaction\n" << std::flush;
   
   
   
    //sound_detector_ = boost::shared_ptr<ALSoundBasedReaction>(new ALSoundBasedReaction(getParentBroker(),"ALSoundBasedReaction"));
    sound_detector_->init();
    std::cout << "DONE" << std::flush << std::endl;
  } catch (AL::ALError& e) {
    throw ALERROR(getName(), "start()", "Impossible to create ALSoundBasedReaction" + e.toString());
  }*/

  initFastAccess();
  dcmWrap_->init();
  if (USE_AL_MOTION)
    al_motion_wrap_->init();
  
  // create sonar
  initSonar();

  // Connect callback to the DCM post proccess
  try {
    std::cout << "NaoInterface::Creating DCM Callback\n" << std::flush;
    dcm_postprocess_connection_ = getParentBroker()->getProxy("DCM")->getModule()->atPostProcess(boost::bind(&naointerface::postProcess, this));
    dcm_preprocess_connection_ = getParentBroker()->getProxy("DCM")->getModule()->atPreProcess(boost::bind(&naointerface::preProcess, this));
  } catch (const AL::ALError &e) {
    throw ALERROR(getName(), "run()", "Error when connecting to DCM postProccess: " + e.toString());
  }

  tts_proxy_->post.stopAll();
  tts_proxy_->setVolume(0.1f);
  tts_proxy_->post.say("Interface");
}

void naointerface::stop() {
}

double naointerface::getSystemTime() {
  double s = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count() / std::micro::den;
  return s;
}

// will be called every 10ms : main cycle
void naointerface::postProcess() {
  // ####### !!!WARNING!!! WE ARE IN REAL-TIME HERE #######
  bool res;
  res = motion_lock_->timed_lock(9);
  if (!res)
    std::cout << "WARNING: Didn't acquire lock in postProcess, but continuing anyway" << std::endl << std::flush;
  // set the frame time and id
  frame_info_->seconds_since_start = getSystemTime() - frame_info_->start_time;
  frame_info_->frame_id++;
  
  populateSensors();

  if (USE_AL_MOTION)
    al_motion_wrap_->getWalkInfo(walk_info_);

  if (res)
    motion_lock_->unlock();
  motion_lock_->notify_one();

  if (speech_->say_text_) {
    speech_->say_text_ = false;
    if ((frame_info_->frame_id - speech_->last_speech_frame_) > 200){
      speech_->last_speech_frame_ = frame_info_->frame_id;
      tts_proxy_->stopAll();
      tts_proxy_->post.say(speech_->text_);
    }
  }
}

void naointerface::preProcess() {
  // ####### !!!WARNING!!! WE ARE IN REAL-TIME HERE #######
  // Execute the commands
  bool res;
  res = motion_lock_->timed_lock(9);
  if (!res)
    std::cout << "WARNING: Didn't acquire lock in preProcess, but continuing anyway" << std::endl << std::flush;
  
  dcmWrap_->sendToActuators(raw_joint_commands_,raw_joint_angles_);
  dcmWrap_->sendToLEDs(led_commands_);

  if (USE_AL_MOTION) {
    al_motion_wrap_->sendWalkParameters(al_walk_param_);
    al_motion_wrap_->sendWalk(walk_request_,walk_info_);
    al_motion_wrap_->sendToActuators(raw_joint_commands_);
  }

  if (res)
    motion_lock_->unlock();
  motion_lock_->notify_one();
}

void naointerface::initMemory() {
  // setup the memory
  MEMORY_INSTANCE = memory_ = new MemoryFrame(true,MemoryOwner::INTERFACE,0,1,true);
  
  memory_->getOrAddBlockByName(frame_info_,"frame_info");
  memory_->getOrAddBlockByName(raw_sensors_,"raw_sensors", MemoryOwner::SHARED);
  memory_->getOrAddBlockByName(raw_joint_angles_,"raw_joint_angles");
  memory_->getOrAddBlockByName(processed_joint_angles_,"processed_joint_angles");
  memory_->getOrAddBlockByName(raw_joint_commands_,"raw_joint_commands");
  memory_->getOrAddBlockByName(processed_joint_commands_,"processed_joint_commands");
  memory_->getOrAddBlockByName(led_commands_,"led_commands",MemoryOwner::VISION);
  memory_->getOrAddBlockByName(al_walk_param_,"al_walk_param");
  memory_->getOrAddBlockByName(walk_request_,"walk_request");
  memory_->getOrAddBlockByName(walk_info_,"walk_info");
  memory_->getOrAddBlockByName(speech_,"speech",MemoryOwner::SHARED);
  memory_->getOrAddBlockByName(robot_state_,"robot_state",MemoryOwner::SHARED);

  // turn on center button to white so we know naoqi is up
  led_commands_->values_[ChestRed]=1.0;
  led_commands_->values_[ChestGreen]=1.0;
  led_commands_->values_[ChestBlue]=1.0;
  
  // set the start time
  frame_info_->source = MEMORY_ROBOT;
  frame_info_->start_time = getSystemTime();

  for (int i = 0; i < NUM_JOINTS; i++)
    processed_joint_commands_->angles_[i] = processed_joint_angles_->values_[i];
  for (int i = 0; i < NUM_JOINTS; i++)
    processed_joint_commands_->stiffness_[i] = -1;
  processed_joint_commands_->body_angle_time_ = 1000;
  processed_joint_commands_->head_pitch_angle_time_ = 1000;
  processed_joint_commands_->head_yaw_angle_time_ = 1000;
  processed_joint_commands_->stiffness_time_ = 1000;
  processed_joint_commands_->send_stiffness_ = false;
  processed_joint_commands_->send_head_pitch_angle_ = false;
  processed_joint_commands_->send_head_yaw_angle_ = false;
}

// ALMemory fast access
void naointerface::initFastAccess() {
  std::cout << "NaoInterface::Initiating Fast Access to Sensors\n" << std::flush;
  sensor_keys_.clear();
  
  // Joints Position list
  for (int i = 0; i < NUM_JOINTS; i++) {
    sensor_keys_.push_back(std::string("Device/SubDeviceList/") + getJointName((Joint)i) + "/Position/Sensor/Value");
  }

  // Sensor Values list
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensor_keys_.push_back(std::string("Device/SubDeviceList/") + getSensorString((Sensor)i) + "/Sensor/Value");
  }
  
  // Sonar Values list
  std::string num;
  for (int i = 0; i < NUM_SONAR_VALS; i++) {
    if (i == 0)
      num = "";
    else
      num = boost::lexical_cast<std::string>(i);
    sensor_keys_.push_back(std::string("Device/SubDeviceList/US/Left/Sensor/Value") + num);
  }
  for (int i = 0; i < NUM_SONAR_VALS; i++) {
    if (i == 0)
      num = "";
    else
      num = boost::lexical_cast<std::string>(i);
    sensor_keys_.push_back(std::string("Device/SubDeviceList/US/Right/Sensor/Value") + num);
  }

  // Joints temperature list
  for (int i = 0; i < NUM_JOINTS; i++) {
    sensor_keys_.push_back(std::string("Device/SubDeviceList/") + getJointName((Joint)i) + "/Temperature/Sensor/Value");
  }

  // Joints stiffness list
  for (int i = 0; i < NUM_JOINTS; i++) {
    sensor_keys_.push_back(std::string("Device/SubDeviceList/") + getJointName((Joint)i) + "/Hardness/Actuator/Value");
  }

  // Create the fast memory access
  fast_sensor_access_->ConnectToVariables(getParentBroker(), sensor_keys_, false);
}

void naointerface::populateSensors(){
  //Get data from the fast sensor proxy
  fast_sensor_access_->GetValues(sensor_values_);

  int offset = 0;

  for (int i = 0; i < NUM_JOINTS; i++)
    raw_joint_angles_->values_[i] = sensor_values_[i + offset];
  raw_joint_angles_->values_[RHipYawPitch] = raw_joint_angles_->values_[LHipYawPitch];
  offset += NUM_JOINTS;

  for (int i = 0; i < NUM_SENSORS; i++)
    raw_sensors_->values_[i] = sensor_values_[i + offset];
  offset += NUM_SENSORS;
 
  for (int i = 0; i < NUM_SONAR_VALS; i++)
    raw_sensors_->sonar_left_[i] = sensor_values_[i + offset];
  offset += NUM_SONAR_VALS;
  for (int i = 0; i < NUM_SONAR_VALS; i++)
    raw_sensors_->sonar_right_[i] = sensor_values_[i + offset];
  offset += NUM_SONAR_VALS;

//  // Debugging sonar
//  std::cout << "Left sonar readings: ";
//  for (int i=0; i<NUM_SONAR_VALS; i++) {
//    std::cout << raw_sensors_->sonar_left_[i] << ",";
//  }
//  std::cout << std::endl;
//  std::cout << "Right sonar readings: ";
//  for (int i=0; i<NUM_SONAR_VALS; i++) {
//    std::cout << raw_sensors_->sonar_right_[i] << ",";
//  }
//  std::cout << std::endl;
 
  for (int i = 0; i < NUM_JOINTS; i++)
    raw_sensors_->joint_temperatures_[i] = sensor_values_[i + offset];
  raw_sensors_->joint_temperatures_[RHipYawPitch] = raw_sensors_->joint_temperatures_[LHipYawPitch];
  offset += NUM_JOINTS;

  for (int i = 0; i < NUM_JOINTS; i++)
    raw_joint_angles_->stiffness_[i] = sensor_values_[i + offset];
  raw_joint_angles_->stiffness_[RHipYawPitch] = raw_joint_angles_->stiffness_[LHipYawPitch];
  offset += NUM_JOINTS;

  //raw_sensors_->values_[accelX] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56
  //raw_sensors_->values_[accelY] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56
  //raw_sensors_->values_[accelZ] *= -9.81 / 56.0; // because aldebaran has them zeroed at 56

  //std::cout << "SENSORS: ";
  //for (int i = 0; i < NUM_SENSORS; i++) 
    //std::cout << raw_sensors_->values_[i] << " ";
  //std::cout << std::endl;
}

void naointerface::startAudioCapture(){
	
}

void naointerface::stopAudioCapture(){
  
}

void naointerface::initSonar(){
  std::cout << "Initializing UltraSound ... " << std::flush;

  try {
    dcmWrap_->initSonar();
  } catch( AL::ALError& e ) {
    std::cout << "FAILED! - " << e.what() << std::endl;
    return;
  }
  std::cout << "Done !" << std::endl;

} 
