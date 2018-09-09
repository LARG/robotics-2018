#include <tool/VisionConfig.h>

VisionConfig::VisionConfig() {
  ball = true;
}

void VisionConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, tab);
  YAML_DESERIALIZE(node, all);
  YAML_DESERIALIZE(node, horizon);
  YAML_DESERIALIZE(node, tooltip);
  YAML_DESERIALIZE(node, calibration);
  YAML_DESERIALIZE(node, checkerboard);
//  YAML_DESERIALIZE(node, ball);
//  YAML_DESERIALIZE(node, goal);
  YAML_DESERIALIZE(node, robot);
  YAML_DESERIALIZE(node, lines);
}

void VisionConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, tab);
  YAML_SERIALIZE(emitter, all);
  YAML_SERIALIZE(emitter, horizon);
  YAML_SERIALIZE(emitter, tooltip);
  YAML_SERIALIZE(emitter, calibration);
  YAML_SERIALIZE(emitter, checkerboard);
//  YAML_SERIALIZE(emitter, ball);
//  YAML_SERIALIZE(emitter, goal);
  YAML_SERIALIZE(emitter, robot);
  YAML_SERIALIZE(emitter, lines);
}
