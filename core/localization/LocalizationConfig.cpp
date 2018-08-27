#include <localization/LocalizationConfig.h>

LocalizationConfig::LocalizationConfig() {
  use_field_edges = false;
  mix_line_intersections = true;
}

void LocalizationConfig::deserialize(const YAML::Node& node) {
  YAML_D(use_field_edges);
  YAML_D(mix_line_intersections);
}

void LocalizationConfig::serialize(YAML::Emitter& emitter) const {
  YAML_S(use_field_edges);
  YAML_S(mix_line_intersections);
}
