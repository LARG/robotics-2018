#ifndef MOTION_MODULE_H
#define MOTION_MODULE_H

#include <Module.h>

#include <memory/BodyModelBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WalkRequestBlock.h>

//struct BodyModelBlock;

class MotionModule: public Module {
 public:
  void specifyMemoryDependency();
  void specifyMemoryBlocks();
  void initSpecificModule();

  void processFrame();
  void processWalkFrame();

 private:
  BodyModelBlock* body_model_;
  JointCommandBlock *commands_;
  JointBlock *joint_angles_;
  SensorBlock *sensors_;
  WalkRequestBlock *walk_request_;

  // UGLY for now
  //WalkEngine walk_engine_;
  //WalkingEngineOutput walk_engine_output_;
};

#endif /* end of include guard: SENSOR_MODULE */
