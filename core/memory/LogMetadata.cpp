#include <memory/LogMetadata.h>

void LogMetadata::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, frames);
  const auto& offsets_node = node["offsets"];
  for(YAML::Iterator it = offsets_node.begin(); it != offsets_node.end(); ++it) {
    int64_t offset;
    *it >> offset;
    offsets.push_back(offset);
  }
}

void LogMetadata::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, frames);
  emitter << YAML::Key << "offsets" << YAML::Value;
  emitter << YAML::BeginSeq;
  for(auto offset : offsets)
    emitter << offset;
  emitter << YAML::EndSeq;
}
