#include <tool/AnnotationConfig.h>

AnnotationConfig::AnnotationConfig() {
}

void AnnotationConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, color);
  YAML_DESERIALIZE(node, selection_type);
  YAML_DESERIALIZE(node, camera);
  YAML_DESERIALIZE(node, auto_create);
}

void AnnotationConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, color);
  YAML_SERIALIZE(emitter, selection_type);
  YAML_SERIALIZE(emitter, camera);
  YAML_SERIALIZE(emitter, auto_create);
}
