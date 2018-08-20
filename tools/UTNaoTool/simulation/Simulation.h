#pragma once

#include <common/Random.h>
#include <common/Roles.h>
#include <memory/MemoryCache.h>
#include <tool/simulation/SimulatedPlayer.h>

class Simulation {
  public:
    virtual bool lmode() { return true; }
    virtual bool complete() { return false; }
    virtual void simulationStep() = 0;
    virtual MemoryCache getGtMemoryCache(int player = 0) = 0;
    virtual MemoryCache getBeliefMemoryCache(int player = 0) { return getGtMemoryCache(player); }
    virtual std::vector<MemoryCache> getPlayerGtMemoryCaches() { 
      auto caches = std::vector<MemoryCache>();
      for(auto i : activePlayers_) {
        const auto cache = getGtMemoryCache(i);
        caches.push_back(cache);
      }
      return caches;
    }
    virtual std::vector<MemoryCache> getPlayerBeliefMemoryCaches() {
      auto caches = std::vector<MemoryCache>();
      for(auto i : activePlayers_)
        caches.push_back(getBeliefMemoryCache(i));
      return caches;
    }
    virtual std::vector<int> activePlayers() { return activePlayers_; }
    virtual std::vector<std::string> getTextDebug(int player = 0) { return std::vector<std::string>(); }
    virtual std::string getSimInfo() { return std::string(); }
    virtual int defaultPlayer() { return CHASER; }
    virtual void moveBall(Point2D position) { };
    virtual void teleportBall(Point2D position) { };
    virtual void movePlayer(Point2D position, float orientation, int player) { };
    virtual void teleportPlayer(Point2D position, float orientation, int player) { };
  protected:
    Random rand_;
    std::vector<int> activePlayers_;
};
