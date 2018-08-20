#ifndef INTERFACEINFO_R3BSIHAC
#define INTERFACEINFO_R3BSIHAC

#include <common/RobotInfo.h>
#include <common/Enum.h>

extern const int robot_joint_signs[NUM_JOINTS];
extern const int spark_joint_signs[NUM_JOINTS];

ENUM(WalkType,
  INVALID_WALK,
  BHUMAN2011_WALK,
  BHUMAN2013_WALK,
  RUNSWIFT2014_WALK
);

extern const bool USE_AL_MOTION;


ENUM(CoreType,
  CORE_ROBOT,
  CORE_SIM,
  CORE_TOOL,
  CORE_TOOLSIM,
  CORE_INIT,
  CORE_TOOL_NO_VISION
);

#endif
