#ifndef TEAM_CONFIG_H
#define TEAM_CONFIG_H

#include <common/RobotConfig.h>
#include <map>

class TeamConfig : public YamlConfig {
  public:
    TeamConfig();

    std::map<int,RobotConfig> robot_configs;
    std::string common_ip, team_broadcast_ip, game_controller_ip;
    bool audio_enabled, svm_enabled;
    int team_udp, team;
    bool optimize_enabled;
  
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
#endif
