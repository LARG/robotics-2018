#pragma once

#include <Eigen/Core>
#include <common/Enum.h>
#include <memory/MemoryBlock.h>
#include <math/Geometry.h>
#include <math/Pose2D.h>
#include <memory/WorldObjectBlock.h>
#define STATE_SIZE 10

#define MAX_MODELS_IN_MEM 4

struct Sides {
  enum SideType {
    NOSIDES,
    LEFTSIDE,
    RIGHTSIDE,
    NUM_SIDES
  };
};

struct Spawns {
  ENUM(SpawnType,
    Ambiguity,
    MultiObject,
    InitialState,
    ReadyState,
    SetState,
    PenalizedState,
    BallFlip,
    Fallen,
    PenaltyKick,
    CenterCircle,
    GoalLines,
    Throwouts
  );
};

DECLARE_INTERNAL_SCHEMA(struct LocalizationBlock : public MemoryBlock {
public:
  SCHEMA_METHODS(LocalizationBlock);
  LocalizationBlock();
  SCHEMA_FIELD(int blueSide);

  SCHEMA_FIELD(int kfType);
  SCHEMA_FIELD(int bestModel);
  SCHEMA_FIELD(float bestAlpha);

  // indicate if there are more with significant likelihood that think we're facing the opposite way
  SCHEMA_FIELD(bool oppositeModels);
  SCHEMA_FIELD(bool fallenModels);
  SCHEMA_FIELD(int numMateFlippedBalls);
  SCHEMA_FIELD(int numBadBallUpdates);

  SCHEMA_FIELD(bool useSR);
  SCHEMA_FIELD(std::array<int,Spawns::NUM_SpawnTypes> spawnFrames);
  SCHEMA_FIELD(std::array<int,MAX_MODELS_IN_MEM> modelNumber);
  SCHEMA_FIELD(std::array<float,MAX_MODELS_IN_MEM> alpha);
  SCHEMA_FIELD(float factor);
  
  SCHEMA_FIELD(std::array<Eigen::Matrix<float, STATE_SIZE, 1, Eigen::DontAlign>, MAX_MODELS_IN_MEM> state);
  SCHEMA_FIELD(std::array<Eigen::Matrix<float, STATE_SIZE, STATE_SIZE, Eigen::DontAlign>, MAX_MODELS_IN_MEM> covariance);
  SCHEMA_FIELD(Eigen::Matrix<float, 2, 2, Eigen::DontAlign> ballSR);
  SCHEMA_FIELD(std::array<Eigen::Matrix<float, 2, 2, Eigen::DontAlign>, NUM_WORLD_OBJS> objectCov);

  Pose2D getPlayerPose(int model);
  Point2D getBallPosition(int model);
  Point2D getBallVelocity(int model);
  Eigen::Matrix2f getPlayerCov(int model);
  Eigen::Matrix2f getBallCov(int model);
  float getOrientationVar(int model);
});
