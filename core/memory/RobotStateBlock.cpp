#include <memory/RobotStateBlock.h>

RobotStateBlock::RobotStateBlock() : WO_SELF(5), team_(TEAM_BLUE), role_(WO_SELF) {
  header.version = 6;
  header.size = sizeof(RobotStateBlock);
  robot_id_ = 0;
  robot_id_ = -1;
  team_changed_ = true;
  ignore_comms_ = false;
  strncpy(reinterpret_cast<char*>(body_id_.data()), "<NO BODY ID SET>", body_id_.size());
  head_version_ = body_version_ = 40;
}
