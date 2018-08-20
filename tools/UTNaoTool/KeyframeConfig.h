#pragma once

#include <common/YamlConfig.h>
#include <common/RobotInfo.h>

class KeyframeConfig : public YamlConfig {
  public:
    KeyframeConfig();
    std::string sequence_file;
    SupportBase base;
    
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;

  private:
    mutable std::string base_s;
};
