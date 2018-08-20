#ifndef ROBOTBEHAVIOR_H
#define ROBOTBEHAVIOR_H

#include <map>

#include <boost/interprocess/sync/scoped_lock.hpp>

#include <memory/MemoryFrame.h>
#include <memory/Lock.h>
#include <memory/CameraBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/SensorBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/ImageBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SimEffectorBlock.h>
//#include <memory/SimImageBlock.h>
#include <memory/SimTruthDataBlock.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/RobotInfoBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/RobotVisionBlock.h>


#include "Behavior.h"
#include "Parser.h"
#include "Random.h"

using namespace std;

class RobotBehavior : public Behavior{
 public:

  RobotBehavior(const std::string teamName, int uNum);
  virtual ~RobotBehavior();
  
  virtual std::string Init();
  virtual std::string Think(const std::string& message);

 public: // Don't asl
  FrameInfoBlock* frame_info_;
  FrameInfoBlock* vision_frame_info_;
  CameraBlock* camera_info_;
  SensorBlock* raw_sensor_block_;
  JointBlock* raw_joint_angles_;
  JointCommandBlock* raw_joint_commands_;
  //SimImageBlock* sim_image_;
  GameStateBlock* game_state_;
  ImageBlock* raw_image_;
  SimEffectorBlock* sim_effectors_;
  SimTruthDataBlock* sim_truth_data_;
  TeamPacketsBlock* team_packets_;
  WorldObjectBlock* world_objects_;
  RobotInfoBlock* robot_info_;
  RobotStateBlock* robot_state_;
  RobotVisionBlock* robot_vision_;

 private:
  string composeAction();
  double computeTorque(const int &effectorID);
  
  void calculateAngles();

  std::string agentTeamName;
  int agentUNum;
  bool init_;
  bool init_beam_;

  Parser* parser_;
  
  MemoryFrame *memory_;
  Lock *motion_lock_;
  Lock *vision_lock_;

  Random random_;
};

#endif // ROBOTBEHAVIOR_H

