#pragma once

#include <common/Random.h>
#include <common/Roles.h>
#include <memory/MemoryCache.h>
#include <tool/simulation/SimulatedPlayer.h>
#include <common/FieldConfiguration.h>

class SimulationPath;

class Simulation {
  public:
    virtual ~Simulation() = default;
    virtual bool lmode() { return true; }
    virtual bool complete() { return false; }
    virtual const SimulationPath* path() const { return nullptr; }
    virtual void simulationStep() = 0;
    virtual MemoryCache getGtMemoryCache(int player = 0) const = 0;
    virtual MemoryCache getBeliefMemoryCache(int player = 0) const { return getGtMemoryCache(player); }
    virtual std::vector<MemoryCache> getPlayerGtMemoryCaches() const { 
      auto caches = std::vector<MemoryCache>();
      for(auto i : activePlayers()) {
        const auto cache = getGtMemoryCache(i);
        caches.push_back(cache);
      }
      return caches;
    }
    virtual std::vector<MemoryCache> getPlayerBeliefMemoryCaches() const {
      auto caches = std::vector<MemoryCache>();
      for(auto i : activePlayers())
        caches.push_back(getBeliefMemoryCache(i));
      return caches;
    }
    virtual std::vector<int> activePlayers() const { return activePlayers_; }
    virtual std::vector<std::string> getTextDebug(int player = 0) { return std::vector<std::string>(); }
    virtual std::string getSimInfo() { return std::string(); }
    virtual int defaultPlayer() const { return CHASER; }
    virtual void moveBall(Point2D position) { };
    virtual void teleportBall(Point2D position) { };
    virtual void movePlayer(Point2D position, float orientation, int player) { };
    virtual void teleportPlayer(Point2D position, float orientation, int player) { };
    virtual bool loadGame(std::string name);
    virtual bool saveGame(std::string name);
    void applyConfig(MemoryCache gtcach, std::vector<MemoryCache> bcaches);
  protected:
    Random rand_;
    std::vector<int> activePlayers_;
    GameConfiguration config_;
};
