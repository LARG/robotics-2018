#include <tool/KeyframeConfig.h>

KeyframeConfig::KeyframeConfig() : base(SupportBase::TorsoBase) {
}

void KeyframeConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, base_s);
  YAML_DESERIALIZE(node, sequence_file);
  base = SupportBaseMethods::fromName(base_s);
}

void KeyframeConfig::serialize(YAML::Emitter& emitter) const {
  if(SupportBaseMethods::valid(base)) {
    base_s = SupportBaseMethods::getName(base);
    YAML_SERIALIZE(emitter, base_s);
  }
  YAML_SERIALIZE(emitter, sequence_file);
}
