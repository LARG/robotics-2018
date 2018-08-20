#pragma once

#include <tool/UTOpenGL/GLDrawer.h>
#include <common/YamlConfig.h>

class SimulatorConfig : public YamlConfig {
  public:
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
    
    std::map<GLDrawer::DisplayOption, bool> options;
  private:
    mutable std::vector<std::string> options_;
};
