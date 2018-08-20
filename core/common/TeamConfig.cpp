#include <common/TeamConfig.h>
#include <communications/CommInfo.h>

TeamConfig::TeamConfig() {
  audio_enabled = true;
  svm_enabled = true;
}

void TeamConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, common_ip);
  YAML_DESERIALIZE(node, audio_enabled);
  YAML_DESERIALIZE(node, svm_enabled);
  YAML_DESERIALIZE(node, robot_configs);
  YAML_DESERIALIZE(node, team_udp);
  YAML_DESERIALIZE(node, team);
  YAML_DESERIALIZE(node, optimize_enabled);
}

void TeamConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, common_ip);
  YAML_SERIALIZE(emitter, audio_enabled);
  YAML_SERIALIZE(emitter, svm_enabled);
  YAML_SERIALIZE(emitter, robot_configs);
  YAML_SERIALIZE(emitter, team_udp);
  YAML_SERIALIZE(emitter, team);
  YAML_SERIALIZE(emitter, optimize_enabled);
}
