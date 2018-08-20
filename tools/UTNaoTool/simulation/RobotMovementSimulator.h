#pragma once

#include <common/Random.h>
#include <memory/MemoryCache.h>
#include <math/Pose2D.h>
#include <math/Geometry.h>

class RobotMovementSimulator {
  public:
    RobotMovementSimulator(int player);
    void step();
    void setCaches(WorldObjectBlock* gtObjects, MemoryCache bcache);
  protected:
    Pose2D getVelocityRequest();
    void resetWalkInfo();

  private:
    Pose2D vel_, maxVel_;
    int player_;
    MemoryCache bcache_;
    WorldObjectBlock* gtObjects_;
    Random rand_;
    bool enableTarget_;
    Point2D ctarget_;
    int rotateTimer_;
    bool rotatePhase_;
};
