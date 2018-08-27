#pragma once

#include <common/YamlConfig.h>

class LogWindowConfig : public YamlConfig {
  public:
    int maxLevel, minLevel, module;
    bool filterTextLogs;

  private:
    void deserialize(const YAML::Node& node) override {
      YAML_DESERIALIZE(node, maxLevel);
      YAML_DESERIALIZE(node, minLevel);
      YAML_DESERIALIZE(node, module);
      YAML_DESERIALIZE(node, filterTextLogs);
    }

    void serialize(YAML::Emitter& emitter) const override {
      YAML_SERIALIZE(emitter, maxLevel);
      YAML_SERIALIZE(emitter, minLevel);
      YAML_SERIALIZE(emitter, module);
      YAML_SERIALIZE(emitter, filterTextLogs);
    }
};
