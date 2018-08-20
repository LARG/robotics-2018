#pragma once

#include <memory/MemoryCache.h>

class Point2D;

class PhysicsSimulator {
  public:
    void setObjects(WorldObjectBlock* objects);
    void step();
    void moveBall(Point2D target);

  private:
    void stepBall();
    WorldObjectBlock* world_object_;
};
