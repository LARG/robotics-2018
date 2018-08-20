#ifndef STATES_H
#define STATES_H

#include <string>
#include <common/Enum.h>
#include <common/YamlConfig.h>
#include <iostream>

enum TEAMS {
 TEAM_BLUE = 0,
 TEAM_RED = 1
};

ENUM(State,
  UNDEFINED_STATE = 0,
  INITIAL = 1,
  READY = 2,
  SET = 3,
  PLAYING = 4,
  TESTING = 5,
  PENALISED = 6,
  FINISHED = 7,
  FALLING = 8,
  BOTTOM_CAM = 9,
  TOP_CAM = 10,
  TEST_ODOMETRY = 11,
  MANUAL_CONTROL = 12
);

const std::string stateNames[] = {
  "undefined",
  "initial",
  "ready",
  "set",
  "playing",
  "testing",
  "penalized",
  "finished",
  "falling",
  "bottom_cam",
  "top_cam",
  "test_odometry",
  "manual_control"
};

ENUM_STREAMS(State);

#endif
