#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <common/YamlConfig.h>
#include <common/InterfaceInfo.h>

class RobotConfig : public YamlConfig {
  public:
    RobotConfig();

    int robot_id, team, self, role;
    float posX, posY, posZ;
    float orientation;
    std::string team_broadcast_ip;
    int team_udp;
    std::string walk_type;
    bool audio_enabled;
    bool svm_enabled;
    bool sonar_enabled;
    
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
#endif
