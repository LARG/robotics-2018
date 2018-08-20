#include <common/GameConfiguration.h>
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

void GameConfiguration::deserialize(const YAML::Node& node) {
  YAML_D(game_state);
  YAML_D(blue_goals);
  YAML_D(red_goals);
  node["roles"] >> s_roles;
  for(auto kvp : s_roles)
    roles[kvp.first] = fromName_Role(kvp.second);
  node["states"] >> s_states;
  for(auto kvp : s_states)
    states[kvp.first] = fromName_State(kvp.second);
  YAML_D(objects);
}

void GameConfiguration::serialize(YAML::Emitter& emitter) const {
  YAML_S(game_state);
  YAML_S(blue_goals);
  YAML_S(red_goals);
  for(auto kvp : roles)
    s_roles[kvp.first] = toName_Role(kvp.second);
  emitter << YAML::Key << "roles" << YAML::Value << s_roles;
  for(auto kvp : states)
    s_states[kvp.first] = toName_State(kvp.second);
  emitter << YAML::Key << "states" << YAML::Value << s_states;
  YAML_S(objects);
}

