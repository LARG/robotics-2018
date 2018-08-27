#ifndef ISOLATED_BEHAVIOR_SIMULATION_H
#define ISOLATED_BEHAVIOR_SIMULATION_H

#include <memory/MemoryCache.h>
#include <VisionCore.h>
#include "Simulation.h"
#include "PhysicsSimulator.h"
#include "CommunicationGenerator.h"

class FieldConfiguration;

class IsolatedBehaviorSimulation : public Simulation {
  public:
    IsolatedBehaviorSimulation(bool locMode = true, int player = 5); 
    virtual void simulationStep();
    inline MemoryCache getGtMemoryCache(int player = 0) const { return gtcache_; }
    inline MemoryCache getBeliefMemoryCache(int player = 0) const { return bcache_; }
    bool lmode() { return lmode_; }
    std::vector<std::string> getTextDebug(int player = 0);
    void moveBall(Point2D pos);
    void teleportBall(Point2D pos);
    void movePlayer(Point2D pos, float orientation, int player);
    void teleportPlayer(Point2D pos, float orientation, int player);
  protected:
    void randomizePlayers(FieldConfiguration&);
    void moveBallRandomly();
    void loadConfig();

    bool lmode_;
    int player_;
    ImageParams iparams_;
    MemoryCache gtcache_, bcache_;
    SimulatedPlayer sim_;
    PhysicsSimulator physics_;
    CommunicationGenerator cg_;
    int approachState_;
    enum ApproachState {
      LONG_RANGE_APPROACH,
      SHORT_RANGE_APPROACH,
      SHORT_RANGE_COMPLETE,
      SHORT_RANGE_WAIT
    };
    int stateTime_;
};

#endif
