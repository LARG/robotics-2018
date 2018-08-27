#pragma once
#include <common/YamlConfig.h>

class LocalizationConfig : public YamlConfig {
  public:
    LocalizationConfig();
    bool use_field_edges;
    bool mix_line_intersections;
  
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
