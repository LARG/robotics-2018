#include <Eigen/Core>
#include <common/YamlConfig.h>

class JointMeasurement : public YamlConfig {
  public:
    bool left; 
    std::vector<float> joints;
    std::vector<Eigen::Vector2f> corners;
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
};

class JointDataset : public YamlConfig, public std::vector<JointMeasurement> {
  public:
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
};
