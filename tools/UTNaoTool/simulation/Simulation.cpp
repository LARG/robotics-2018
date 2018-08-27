#include <tool/simulation/Simulation.h>
#include <common/Util.h>
#include <memory/GameStateBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/WorldObjectBlock.h>
#include <common/tinyformat.h>

bool Simulation::loadGame(std::string name) {
  auto path = util::format("%s/%s.yaml", util::cfgpath(util::FieldConfigs), name);
  return config_.load(path);
}

bool Simulation::saveGame(std::string name) {
  auto path = util::format("%s/%s.yaml", util::cfgpath(util::FieldConfigs), name);
  return config_.save(path);
}

void Simulation::applyConfig(MemoryCache gtcache, vector<MemoryCache> bcaches) {
  config_.objects.gtconfig.place(gtcache.world_object);
  for(auto& bcache : bcaches)
    config_.objects.bconfig.place(bcache.world_object);
  auto all = bcaches;
  all.insert(all.end(), gtcache);
  for(auto& cache : all) {
    if(cache.game_state != nullptr) {
      cache.game_state->ourScore = config_.blue_goals;
      cache.game_state->opponentScore = config_.red_goals;
      auto state = config_.game_state;
      if(cache.game_state != nullptr && state != UNDEFINED) {
        cache.game_state->setState(state);
      }
    }
    for(int i = WO_TEAM_FIRST; i <= WO_TEAM_LAST; i++) {
      auto role = config_.role(i);
      if(cache.robot_state != nullptr && role != UNDEFINED)
        cache.robot_state->role_ = role;
      auto state = config_.state(i);
      if(state == UNDEFINED) state = config_.game_state;
      if(cache.game_state != nullptr && state != UNDEFINED) {
        cache.game_state->setState(state);
      }
    }
  }
}
