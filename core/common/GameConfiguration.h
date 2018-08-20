#pragma once

#include <common/YamlConfig.h>
#include <math/Geometry.h>
#include <common/Roles.h>
#include <common/States.h>

class WorldObjectBlock;

class ObjectConfiguration : public YamlConfig {
  public:
    Point2D loc;
    float orientation;
    float height;

    ObjectConfiguration() = default;
    ObjectConfiguration(float x, float y, float orientation = 0, float height = 0) : loc(x,y), orientation(orientation), height(height) {
    }

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

class FieldConfiguration : public YamlConfig {
  public:
    FieldConfiguration() { }
    FieldConfiguration(std::initializer_list<std::pair<int,ObjectConfiguration>> l);
    void place(WorldObjectBlock* world_object) const;
    void see(WorldObjectBlock* world_object) const;
    const FieldConfiguration& operator=(std::initializer_list<std::pair<int,ObjectConfiguration>> l) {
      return *this = FieldConfiguration(l);
    }
    inline ObjectConfiguration& operator[](int key) { return placements_[key]; }
    inline bool contains(int key) const { return placements_.find(key) != placements_.end(); }
      
  private:
    std::map<int,ObjectConfiguration> placements_;
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

class BeliefConfiguration : public YamlConfig {
  public:
    FieldConfiguration gtconfig, bconfig;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

class GameConfiguration : public YamlConfig {
  public:
    State game_state = INITIAL;
    int blue_goals = 0, red_goals = 0;
    std::map<int,Role> roles;
    std::map<int,State> states;
    BeliefConfiguration objects;
    inline Role role(int player) {
      auto it = roles.find(player);
      if(it == roles.end()) return Role::UNDEFINED;
      return it->second;
    }
    inline State state(int player) {
      auto it = states.find(player);
      if(it == states.end()) return State::UNDEFINED_STATE;
      return it->second;
    }

  private:
    mutable std::map<int,std::string> s_roles;
    mutable std::map<int,std::string> s_states;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

