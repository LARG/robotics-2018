#include <common/RobotConfig.h>
#include <common/WorldObject.h>
#include <communications/CommInfo.h>

RobotConfig::RobotConfig() {
  self = WO_TEAM_LAST;
  team = 1;
  role = 5;
  robot_id = 0;
  posX = posY = posZ = orientation = 0;
  team_udp = 10001;
  team_broadcast_ip = "10.202.16.255";
  walk_type = getName(RUNSWIFT2014_WALK);
  team = 1;
  audio_enabled = true;
  svm_enabled = true;
}

void RobotConfig::deserialize(const YAML::Node& node) {
  YAML_D(robot_id);
  YAML_D(team);
  YAML_D(self);
  YAML_D(posX);
  YAML_D(posY);
  YAML_D(posZ);
  YAML_D(orientation);
  YAML_D(team_udp);
  YAML_D(team_broadcast_ip);
  YAML_D(walk_type);
  YAML_D(audio_enabled);
  YAML_D(svm_enabled);
  YAML_D(sonar_enabled);
}

void RobotConfig::serialize(YAML::Emitter& emitter) const {
  YAML_S(robot_id);
  YAML_S(team);
  YAML_S(self);
  YAML_S(posX);
  YAML_S(posY);
  YAML_S(posZ);
  YAML_S(orientation);
  YAML_S(team_udp);
  YAML_S(team_broadcast_ip);
  YAML_S(walk_type);
  YAML_S(audio_enabled);
  YAML_S(svm_enabled);
  YAML_S(sonar_enabled);
}
