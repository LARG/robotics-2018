#ifndef SPECIAL_MOTION_MODULE_H
#define SPECIAL_MOTION_MODULE_H

#include <string>
#include <iostream>
#include <Module.h>
#include <common/RobotInfo.h>
#include <vector>
#include <motion/SpecialMotionParser.h>

#include <common/Enum.h>

class FrameInfoBlock;
class WalkRequestBlock;
class JointCommandBlock;
class OdometryBlock;
class JointBlock;
class SensorBlock;
class BodyModelBlock;
class KickRequestBlock;

class SpecialMotionModule: public Module {
public:
  ENUM (Motion,
    Getup,
    standUpBackNao,
    standUpFrontNao,
    backFreeArms
    //kickSidewardsNao,
    //kickDiagonalNao
  );

  ENUM (State,
    INITIAL,
    EXECUTE,
    WAIT,
    FINISHED,
    // from getup
    STIFFNESS_OFF,
    PREPARE_ARMS,
    STIFFNESS_ON,
    CROSS,
    STAND,
    FREE_ARMS,
    PAUSE_BEFORE_RESTART,
    // don't use this state except for testing, never changes states
    NO_ESCAPE
  );

public:
  SpecialMotionModule();
  void initSpecificModule();
  void startMotion(Motion m);

  void specifyMemoryDependency();
  void specifyMemoryBlocks();

  void processFrame();
  bool isDoingSpecialMotion() { return (state != INITIAL) && (state != FINISHED); };

protected:

  virtual bool processFrameChild() {return false;} // return true if it's handled execution this frame
  bool isValueValid(float val);
  bool isVoidNum(float val);
  void processJointCommands(float time, float angles[NUM_JOINTS]);
  void processJointHardness(float time,float stiffness[NUM_JOINTS]);
  void startMotionSequence();
  void executeMotionSequence();
  void loadMotionFiles();
  int getCurrentFrame();
//   int jointMapping[NUM_JOINTS];

  FrameInfoBlock *frame_info_;
  JointCommandBlock* commands_;
  WalkRequestBlock* walk_request_;
  OdometryBlock* odometry_;
  JointBlock *joint_angles_;
  SensorBlock *sensors_;
  BodyModelBlock *body_model_;
  KickRequestBlock *kick_request_;
  
  std::string teamID;

  void transitionToState(State nstate);
  float getTimeInState();
  bool isMotionDoneExecuting() {return getTimeInState() > motionSeqList[currMotion].back().time;}

  static std::vector<std::vector<SpecialMotion> > motionSeqList;
  Motion currMotion;
  State state;
  float stateStartTime;
  float storedAngles[NUM_JOINTS];
  float storedCommands[NUM_JOINTS];
};

#endif /* end of include guard: GETUP_MODULE */
