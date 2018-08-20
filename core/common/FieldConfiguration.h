#pragma once

#include <common/YamlConfig.h>
#include <initializer_list>
#include <math/Geometry.h>

class WorldObjectBlock;

class ObjectConfiguration : public YamlConfig {
  public:
    Point2D loc;
    float orientation;
    float height;

    ObjectConfiguration() = default;
    ObjectConfiguration(float x, float y, float orientation = 0, float height = 0) : loc(x,y), orientation(orientation), height(height) {
    }
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
};

class FieldConfiguration : public YamlConfig {
  public:
    FieldConfiguration() { }
    FieldConfiguration(std::initializer_list<std::pair<int,ObjectConfiguration>> l);
    void place(WorldObjectBlock* world_object) const;
    void see(WorldObjectBlock* world_object) const;
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
    const FieldConfiguration& operator=(std::initializer_list<std::pair<int,ObjectConfiguration>> l) {
      return *this = FieldConfiguration(l);
    }
    inline ObjectConfiguration& operator[](int key) { return placements_[key]; }
      
  private:
    std::map<int,ObjectConfiguration> placements_;
};

class BeliefConfiguration : public YamlConfig {
  public:
    FieldConfiguration gtconfig, bconfig;
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
};
