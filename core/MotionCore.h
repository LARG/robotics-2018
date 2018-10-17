#ifndef MOTION_CORE_2H7QHCY7
#define MOTION_CORE_2H7QHCY7

#include <memory/MemoryFrame.h>
#include <memory/LogWriter.h>
#include <memory/TextLogger.h>
#include <common/InterfaceInfo.h> // for core_type


class BodyModelBlock;
class FrameInfoBlock;
class JointTarget; // for head request
class JointBlock;
class JointCommandBlock;
class KickParamBlock;
class KickRequestBlock;
class OdometryBlock;
class SensorBlock;
class WalkParamBlock;
class ALWalkParamBlock;
class WalkRequestBlock;
class WalkResponseBlock;
class ProcessedSonarBlock;
class WalkInfoBlock;
class RobotInfoBlock;
//class WorldObjectBlock; //for rswalk2014
class RobotStateBlock;

class KinematicsModule;
class KickModule;
class MotionModule;
class GetupModule;
class SpecialMotionModule;
class SensorModule;
class SonarModule;
class WalkModule;
class RobotCalibration;

class MotionCore {
public:
  MotionCore(CoreType type, bool use_shared_memory,int team_num, int player_num);
  ~MotionCore();
  
  void preProcess();
  void postProcess();
  bool alreadyProcessedFrame();
  void processMotionFrame();
  void processSensorUpdate();

  void logMemory();
  void enableLogging(const char *filename = NULL);
  void enableTextLogging(const char *filename = NULL);
  void disableLogging();
  void disableTextLogging();
  
  MemoryFrame memory_;
  CoreType type_;
  unsigned int last_frame_processed_;

  bool use_com_kick_;

  KinematicsModule *kinematics_;
  KickModule *kick_;
  MotionModule *motion_;
  SensorModule *sensor_;
  SonarModule *sonar_;
  GetupModule *getup_;
  SpecialMotionModule *specialM_;
  WalkModule *walk_;

  std::unique_ptr<TextLogger> textlog_;

  static MotionCore *inst_;
  
  FrameInfoBlock *frame_info_;
  // joint angles
  JointBlock* raw_joint_angles_;
  JointBlock* processed_joint_angles_;
  // commands
  JointCommandBlock* raw_joint_commands_;
  JointCommandBlock* processed_joint_commands_;
  // sensors
  SensorBlock* raw_sensors_;
  SensorBlock* processed_sensors_;
  
  BodyModelBlock* body_model_;
  KickRequestBlock* kick_request_;
  OdometryBlock* odometry_;
  KickParamBlock* kick_params_;
  WalkParamBlock* walk_param_;
  ALWalkParamBlock* al_walk_param_;
  WalkRequestBlock* walk_request_;
  WalkResponseBlock* walk_response_;
  ProcessedSonarBlock *processed_sonar_;
  WalkInfoBlock* walk_info_;
  RobotInfoBlock* robot_info_;
  //WorldObjectBlock* world_objects_;//for rswalk2014
  RobotStateBlock* robot_state_;

  void publishData();
  void receiveData();
 
private: 
  // synchronized data
  BodyModelBlock *sync_body_model_;
  JointBlock *sync_joint_angles_;
  KickRequestBlock *sync_kick_request_;
  OdometryBlock *sync_odometry_;
  SensorBlock *sync_sensors_;
  WalkRequestBlock *sync_walk_request_;
  WalkResponseBlock *sync_walk_response_;
  JointCommandBlock *sync_joint_commands_;
  ProcessedSonarBlock *sync_processed_sonar_;
  KickParamBlock *sync_kick_params_;
  WalkParamBlock *sync_walk_param_;
  ALWalkParamBlock *sync_al_walk_param_;
  WalkInfoBlock* sync_walk_info_;

  double fps_time_;
  unsigned int fps_frames_processed_;
  float time_motion_started_;
  RobotCalibration* calibration_;

private:

  void init();
  void initMemory();
  void initModules();
  void initWalkEngine();
  void setMemoryVariables();
  void loadCalibration();

  unsigned int last_stand_frame_;
  unsigned int next_stand_frame_;
  unsigned int last_odometry_update_;
};

#endif /* end of include guard: CORE_2H7QHCY7 */
