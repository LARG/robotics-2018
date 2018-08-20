#ifndef GETUP_MODULE_H
#define GETUP_MODULE_H

#include <string>
#include <iostream>
#include <Module.h>
#include <common/RobotInfo.h>
#include <vector>

#include <motion/SpecialMotionModule.h>
#include <memory/OdometryBlock.h>
#include <common/RingBufferWithSum.h>

class GetupModule: public SpecialMotionModule {
public:
  GetupModule();

  bool isGettingUp() {return isDoingSpecialMotion();}
  void initGetup();
  void processGetup();
  bool needsStand() {return state == STAND;}

protected:
  bool processFrameChild();
  bool areJointsHot();
  void setBackGetup();
  void setFrontGetup();
  void prepareArms();
  void setJointFromOffset(float angles[], Joint joint, float offset);
  bool armsStuckBehindBack();
  void selectGetup();

  bool shouldAbortGetup();

private:
  void cross();
  void prepareFall(float diffT);
  Getup::GetupType getUpSide;
  int numCrosses;
  int numRestarts;

  RingBufferWithSum<float,20> angleYBuffer;
};

#endif /* end of include guard: GETUP_MODULE */
