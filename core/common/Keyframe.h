#pragma once

#include <common/RobotInfo.h>
#include <common/YamlConfig.h>
#include <cstring>

class Keyframe : public YamlConfig {
  public:
    Keyframe(std::string name="");
    Keyframe(std::array<float, NUM_JOINTS> joints, int frames);
    std::array<float,NUM_JOINTS> joints;
    std::string name;
    int frames;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

class KeyframeSequence : public YamlConfig {
  public:
    std::vector<Keyframe> keyframes;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
