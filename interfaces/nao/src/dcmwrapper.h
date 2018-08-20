#ifndef DCM_WRAPPER_H
#define DCM_WRAPPER_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <memory/JointCommandBlock.h>
#include <memory/JointBlock.h>

class LEDBlock;

namespace AL
{
  class DCMProxy;
  class ALValue;
}

class DCMWrapper {
 public:
  DCMWrapper();
  ~DCMWrapper();

  // Set the proxy to the DCM module
  void setProxy(boost::shared_ptr<AL::DCMProxy> pr);

  // Intialise the DCM
  void init();

  // Send the commands
  void sendToActuators(JointCommandBlock *raw_joint_commands, JointBlock *raw_joint_angles_);
  void sendToLEDs(LEDBlock *led_coomands);

  void frontGetup();
  void backGetup();

  void initSonar();
  void testSonar();

 private:
  void initModes();
  void initAliases();
  void initAlias(const std::string &name, const std::string &deviceSuffix, int jointIndStart, int numJoints);
  void initCommands(AL::ALValue &commands, const std::string &commandType, int numJoints);
  void initSonarCommands();
  void initLEDCommands();
  void initHands();

  void sendJointCommands(bool send, AL::ALValue &commands, float time, float angles[NUM_JOINTS], int jointIndStart, int numJoints);
  std::vector<std::string> dcm_modes_;
  
  // Used to store commands to send
  AL::ALValue led_commands_;
  AL::ALValue body_position_commands_;  
  AL::ALValue head_pitch_commands_;
  AL::ALValue head_yaw_commands_;
  AL::ALValue stiffness_commands_;
  AL::ALValue sonar_commands_;

  static const std::string body_position_name_;
  static const std::string head_pitch_name_;
  static const std::string head_yaw_name_;
  static const std::string stiffness_name_;

  boost::shared_ptr<AL::DCMProxy> dcm_proxy_;
  bool initialized_;
};

#endif
