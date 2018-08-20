#include <common/FieldConfiguration.h>
#include <memory/WorldObjectBlock.h>

void ObjectConfiguration::deserialize(const YAML::Node& node) {
  node["x"] >> loc.x;
  node["y"] >> loc.y;
  YAML_DESERIALIZE(node, orientation);
  orientation *= DEG_T_RAD;
  YAML_DESERIALIZE(node, height);
};

void ObjectConfiguration::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::Key << "x" << YAML::Value << loc.x;
  emitter << YAML::Key << "y" << YAML::Value << loc.y;
  emitter << YAML::Key << "orientation" << YAML::Value << (int)(orientation * RAD_T_DEG);
  YAML_SERIALIZE(emitter, height);
}

FieldConfiguration::FieldConfiguration(std::initializer_list<std::pair<int,ObjectConfiguration>> l) {
  for(auto p : l)
    placements_[p.first] = p.second;
}

void FieldConfiguration::place(WorldObjectBlock* world_object) const {
  for(int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++)
    world_object->objects_[i].loc = Point2D(10000,10000);
  for(auto kvp : placements_) {
    auto& obj = world_object->objects_[kvp.first];
    obj.loc = kvp.second.loc;
    obj.orientation = kvp.second.orientation;
    obj.height = kvp.second.height;
  }
}

void FieldConfiguration::see(WorldObjectBlock* world_object) const {
  for(auto kvp : placements_)
    world_object->objects_[kvp.first].seen = true;
}

void FieldConfiguration::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, placements_);
};

void FieldConfiguration::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, placements_);
}

void BeliefConfiguration::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, gtconfig);
  YAML_DESERIALIZE(node, bconfig);
}

void BeliefConfiguration::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, gtconfig);
  YAML_SERIALIZE(emitter, bconfig);
}
