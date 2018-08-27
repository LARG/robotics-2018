#ifndef SIM_PLAYER_H
#define SIM_PLAYER_H

#include <vector>
#include <string>

#include <memory/MemoryCache.h>
#include <math/Pose2D.h>
#include <math/Geometry.h>
#include <common/RobotInfo.h>

#include <tool/simulation/ObservationGenerator.h>
#include <tool/simulation/RobotMovementSimulator.h>

#include <common/Roles.h>

class VisionCore;
class MemoryFrame;

class SimulatedPlayer {

 public:
  static bool DEBUGGING_POSITIONING, ALLOW_FALLING;

  SimulatedPlayer(int team, int self, bool lMode = false);
  ~SimulatedPlayer();
  void initLocalization();
  void setRole(Role role);
  void setFallen();
  void resetCounters();
  inline MemoryFrame* getMemory() const {
    return memory_;
  }
  inline MemoryCache getMemoryCache() const {
    return cache_;
  }
  void setPenalty(WorldObjectBlock* simMem);
  bool processFrame();
  bool processFrame(WorldObjectBlock* wo, GameStateBlock* gs);
  std::vector<std::string> getTextDebug();
  
  MemoryCache cache_;
  bool PRINT;
  int team_;
  int self_;
  VisionCore* core;

 private:
  void init(int team, int self, MemoryFrame* memory);

  void setMemory(MemoryFrame* memory);
  void updateMemoryBlocks();
  void setStrategy();
  void setPenaltyPosition(WorldObjectBlock* simMem);

  MemoryFrame* memory_;
  float penaltySeconds;
  float kickSeconds;
  float kickHitSeconds;
  float getupSeconds;
  int diving;
  float roll;
  float dt;
  bool locMode;
  
  // noise factors
  float visionErrorFactor;//0.0;
  float missedObsFactor;//0.0;
  float odometryErrorFactor;
  float kickErrorFactor;
  
  // kick, walk, getup speeds
  float getupTimeLength;
  float kickFullTime;
  float kickHitTime;
  float maxFwdVel;
  float maxSideVel;
  float maxTurnVel;

  // to target
  bool walkToTarget;
  Point2D target;

  // walk info
  Pose2D absWalkVel;
  Pose2D relWalkVel;

  float panStopTime;
  bool panMoving;

  void updateBasicInputs(WorldObjectBlock* simulationMem, GameStateBlock *gameState);
  bool updateOutputs(WorldObjectBlock* simulationMem);

  float crop(float v, float min, float max);
  ImageParams iparams_;
  ObservationGenerator og_;
  RobotMovementSimulator rmsim_;
};


#endif
