#pragma once

#include <common/Enum.h>
#include <common/Poses.h>
#include <common/RobotInfo.h>
#include <common/WorldObject.h>
#include <array>

#define __TOOL_PACKET_DATA_LENGTH 1024

struct ToolPacket {
  static const int DATA_LENGTH;
  ENUM(MessageType,
    None,
    StateInitial,
    StateReady,
    StateSet,
    StatePlaying,
    StatePenalized,
    StateFinished,
    StateTesting,
    StateTestOdometry,
    StateCameraTop,
    StateCameraBottom,
    LogSelect,
    LogBegin,
    LogEnd,
    LogComplete,
    StreamBegin,
    StreamEnd,
    RestartInterpreter,
    SetTopCameraParameters,
    SetBottomCameraParameters,
    GetCameraParameters,
    ResetCameraParameters,
    ManualControl,
    RunBehavior,
    SetStiffness    
  );
  MessageType message;
  int frames;
  float interval;
  struct {
    float x, y, theta, time;
    Poses::Stance stance;
  } odom_command;
  std::array<char, __TOOL_PACKET_DATA_LENGTH> data;
  std::array<float, NUM_JOINTS> jointStiffness;
  std::array<bool, NUM_WorldObjectTypes> requiredObjects;
  bool hasRequiredObjects;
  ToolPacket();
  ToolPacket(MessageType message);
  ToolPacket(MessageType message, std::string data);
};
