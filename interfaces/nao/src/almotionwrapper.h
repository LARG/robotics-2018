#ifndef ALMOTIONWRAPPER_NC3FGGCA
#define ALMOTIONWRAPPER_NC3FGGCA

#include <alvalue/alvalue.h>
#include <boost/shared_ptr.hpp>
#include <math/Pose2D.h>

class JointCommandBlock;
class WalkInfoBlock;
class ALWalkParamBlock;
class WalkRequestBlock;


namespace AL
{
  class ALMotionProxy;
}

class ALMotionWrapper {
public:
  ALMotionWrapper();
  ~ALMotionWrapper();

  void setProxy(boost::shared_ptr<AL::ALMotionProxy> pr);
  void init();

  void sendWalk(WalkRequestBlock *walk_request, WalkInfoBlock *walk_info);
  void sendWalkParameters(ALWalkParamBlock *params);
  void sendToActuators(JointCommandBlock *raw_joint_commands);
  void getWalkInfo(WalkInfoBlock *walk_info);
  void CarlosBackStandup();

private:
  boost::shared_ptr<AL::ALMotionProxy> motion_proxy_;

  AL::ALValue body_joint_names_;
  AL::ALValue leg_joint_names_;
  AL::ALValue body_position_time_;
  AL::ALValue body_position_commands_;

  AL::ALValue arm_joint_names_;
  AL::ALValue arm_position_commands_;

  AL::ALValue head_pitch_joint_names_;
  AL::ALValue head_yaw_joint_names_;

  AL::ALValue head_position_time_;
  AL::ALValue head_position_commands_;
  
  AL::ALValue stiffness_joint_names_;
  AL::ALValue stiffness_time_;
  AL::ALValue stiffness_commands_;

  AL::ALValue walk_params_;

  Pose2D max_vels_;
};

#endif /* end of include guard: ALMOTIONWRAPPER_NC3FGGCA */
