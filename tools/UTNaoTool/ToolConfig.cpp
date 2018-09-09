#include "ToolConfig.h"

ToolConfig::ToolConfig() {
  fullProcess = true;
  onDemand = false;
  streaming = false;
  enableAudio = false;
}

ToolConfig::~ToolConfig() {
}

void ToolConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, filesLocationIndex);
  YAML_DESERIALIZE(node, filesUpdateTimeChecked);
  // YAML_DESERIALIZE(node, streaming);
  YAML_DESERIALIZE(node, onDemand);
  YAML_DESERIALIZE(node, bypassVision);
  YAML_DESERIALIZE(node, visionOnly);
  YAML_DESERIALIZE(node, fullProcess);
  YAML_DESERIALIZE(node, logStream);
  YAML_DESERIALIZE(node, logFrame);
  YAML_DESERIALIZE(node, logStep);
  YAML_DESERIALIZE(node, logStart);
  YAML_DESERIALIZE(node, logEnd);
  YAML_DESERIALIZE(node, logPath);
  YAML_DESERIALIZE(node, coreBehaviors);
  YAML_DESERIALIZE(node, logWindowConfig);
  YAML_DESERIALIZE(node, worldConfig);
  YAML_DESERIALIZE(node, rcConfig);
  YAML_DESERIALIZE(node, kfConfig);
  YAML_DESERIALIZE(node, visionConfig);
  YAML_DESERIALIZE(node, annotationConfig);
  YAML_DESERIALIZE(node, memorySelectConfig);
  YAML_DESERIALIZE(node, loggingModules);
  YAML_DESERIALIZE(node, enableAudio);
}

void ToolConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, filesLocationIndex);
  YAML_SERIALIZE(emitter, filesUpdateTimeChecked);
  // YAML_SERIALIZE(emitter, streaming);
  YAML_SERIALIZE(emitter, onDemand);
  YAML_SERIALIZE(emitter, bypassVision);
  YAML_SERIALIZE(emitter, visionOnly);
  YAML_SERIALIZE(emitter, fullProcess);
  YAML_SERIALIZE(emitter, logStream);
  YAML_SERIALIZE(emitter, logFrame);
  YAML_SERIALIZE(emitter, logStep);
  YAML_SERIALIZE(emitter, logStart);
  YAML_SERIALIZE(emitter, logEnd);
  YAML_SERIALIZE(emitter, logPath);
  YAML_SERIALIZE(emitter, coreBehaviors);
  YAML_SERIALIZE(emitter, logWindowConfig);
  YAML_SERIALIZE(emitter, worldConfig);
  YAML_SERIALIZE(emitter, rcConfig);
  YAML_SERIALIZE(emitter, kfConfig);
  YAML_SERIALIZE(emitter, visionConfig);
  YAML_SERIALIZE(emitter, annotationConfig);
  YAML_SERIALIZE(emitter, memorySelectConfig);
  YAML_SERIALIZE(emitter, loggingModules);
  YAML_SERIALIZE(emitter, enableAudio);
}
