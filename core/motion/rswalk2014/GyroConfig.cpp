#include "GyroConfig.h"

GyroConfig::GyroConfig() {
  offsetX = 0.0;
  offsetY = 0.0;
  offsetZ = 0.0;
  calibration_write_time = -1.0;
}

void GyroConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, offsetX);
  YAML_DESERIALIZE(node, offsetY);
  YAML_DESERIALIZE(node, calibration_write_time);
}

void GyroConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, offsetX);
  YAML_SERIALIZE(emitter, offsetY);
  YAML_SERIALIZE(emitter, calibration_write_time);
}
