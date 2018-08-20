#include <tool/SimulatorConfig.h>

void SimulatorConfig::deserialize(const YAML::Node& node) {
  node["options"] >> options_;
  for(int i = 0; i < (int)GLDrawer::NUM_DisplayOptions; i++)
    options[(GLDrawer::DisplayOption)i] = false;
  for(auto option : options_)
    options[GLDrawer::fromName_DisplayOption(option)] = true;
}

void SimulatorConfig::serialize(YAML::Emitter& emitter) const {
  options_.clear();
  for(auto kvp : options)
    if(kvp.second)
      options_.push_back(GLDrawer::getName(kvp.first));
  emitter << YAML::Key << "options" << YAML::Value << options_;
}
