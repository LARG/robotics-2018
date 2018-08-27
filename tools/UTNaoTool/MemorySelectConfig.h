#pragma once

#include <common/YamlConfig.h>
#include <map>

class MemorySelectConfig : public YamlConfig {
  public:
    MemorySelectConfig();
    std::map<std::string,bool> selected_blocks;
    std::vector<std::string> additional_blocks;
    
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
