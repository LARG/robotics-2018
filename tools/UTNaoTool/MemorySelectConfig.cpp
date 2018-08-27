#include <tool/MemorySelectConfig.h>

MemorySelectConfig::MemorySelectConfig() {
  additional_blocks = {
    "sim_truth_data", "raw_sensors", "behavior_trace"
  };
}

void MemorySelectConfig::deserialize(const YAML::Node& node) {
  YAML_D(selected_blocks);
  YAML_D(additional_blocks);
}

void MemorySelectConfig::serialize(YAML::Emitter& emitter) const {
  YAML_S(selected_blocks);
  YAML_S(additional_blocks);
}
