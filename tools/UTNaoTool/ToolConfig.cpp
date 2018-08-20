#include "ToolConfig.h"

ToolConfig::ToolConfig() {
  locAndVis = true;
  onDemand = false;
  streaming = false;
  enableAudio = false;
}

void ToolConfig::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, logLevelLow);
  YAML_DESERIALIZE(node, logLevelHigh);
  YAML_DESERIALIZE(node, moduleTypeIndex);
  YAML_DESERIALIZE(node, filesLocationIndex);
  YAML_DESERIALIZE(node, filesUpdateTimeChecked);
  YAML_DESERIALIZE(node, streaming);
  YAML_DESERIALIZE(node, onDemand);
  YAML_DESERIALIZE(node, locOnly);
  YAML_DESERIALIZE(node, visOnly);
  YAML_DESERIALIZE(node, locAndVis);
  YAML_DESERIALIZE(node, logStream);
  YAML_DESERIALIZE(node, logFrame);
  YAML_DESERIALIZE(node, logStep);
  YAML_DESERIALIZE(node, logStart);
  YAML_DESERIALIZE(node, logEnd);
  YAML_DESERIALIZE(node, logFile);
  YAML_DESERIALIZE(node, coreBehaviors);
  YAML_DESERIALIZE(node, worldConfig);
  YAML_DESERIALIZE(node, rcConfig);
  YAML_DESERIALIZE(node, kfConfig);
  YAML_DESERIALIZE(node, loggingModules);
  YAML_DESERIALIZE(node, enableAudio);
}

void ToolConfig::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, logLevelLow);
  YAML_SERIALIZE(emitter, logLevelHigh);
  YAML_SERIALIZE(emitter, moduleTypeIndex);
  YAML_SERIALIZE(emitter, filesLocationIndex);
  YAML_SERIALIZE(emitter, filesUpdateTimeChecked);
  YAML_SERIALIZE(emitter, streaming);
  YAML_SERIALIZE(emitter, onDemand);
  YAML_SERIALIZE(emitter, locOnly);
  YAML_SERIALIZE(emitter, visOnly);
  YAML_SERIALIZE(emitter, locAndVis);
  YAML_SERIALIZE(emitter, logStream);
  YAML_SERIALIZE(emitter, logFrame);
  YAML_SERIALIZE(emitter, logStep);
  YAML_SERIALIZE(emitter, logStart);
  YAML_SERIALIZE(emitter, logEnd);
  YAML_SERIALIZE(emitter, logFile);
  YAML_SERIALIZE(emitter, coreBehaviors);
  YAML_SERIALIZE(emitter, worldConfig);
  YAML_SERIALIZE(emitter, rcConfig);
  YAML_SERIALIZE(emitter, kfConfig);
  YAML_SERIALIZE(emitter, loggingModules);
  YAML_SERIALIZE(emitter, enableAudio);
}
