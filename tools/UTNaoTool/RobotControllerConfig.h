#ifndef ROBOT_CONTROLLER_CONFIG_H
#define ROBOT_CONTROLLER_CONFIG_H

#include <common/YamlConfig.h>

class RobotControllerConfig : public YamlConfig {
  public:
    float accelX, accelY, accelTheta, maxVelX, maxVelY, maxVelTheta;
    
    void deserialize(const YAML::Node& node) {
      YAML_DESERIALIZE(node, accelX);
      YAML_DESERIALIZE(node, accelY);
      YAML_DESERIALIZE(node, accelTheta);
      YAML_DESERIALIZE(node, maxVelX);
      YAML_DESERIALIZE(node, maxVelY);
      YAML_DESERIALIZE(node, maxVelTheta);
    }

    void serialize(YAML::Emitter& emitter) const {
      YAML_SERIALIZE(emitter, accelX);
      YAML_SERIALIZE(emitter, accelY);
      YAML_SERIALIZE(emitter, accelTheta);
      YAML_SERIALIZE(emitter, maxVelX);
      YAML_SERIALIZE(emitter, maxVelY);
      YAML_SERIALIZE(emitter, maxVelTheta);
    }

};

#endif
