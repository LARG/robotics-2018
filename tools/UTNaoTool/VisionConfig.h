#pragma once

#include <common/YamlConfig.h>

class VisionConfig : public YamlConfig {
  public:
    VisionConfig();
    int tab;
    bool 
      all, horizon, tooltip, calibration, checkerboard,
      ball, goal, robot, lines;
    
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
