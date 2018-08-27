#include <Eigen/Core>
#include <common/YamlConfig.h>

class JointMeasurement : public YamlConfig {
  public:
    bool left; 
    std::vector<float> joints;
    std::vector<Eigen::Vector2f> corners;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

class JointDataset : public YamlConfig, public std::vector<JointMeasurement> {
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
