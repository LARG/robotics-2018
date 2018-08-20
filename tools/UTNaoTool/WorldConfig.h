#ifndef WORLD_CONFIG_H
#define WORLD_CONFIG_H

#include <common/YamlConfig.h>

class WorldConfig : public YamlConfig {
  public:
    int simPlayers;
    double playSpeed;
    int mode;
    bool autoplay;
    int teamNumber;
    int teamColor;
    std::string locSimPathfile;
    int skip;
    int seed;

    WorldConfig() : 
      simPlayers(1),
      playSpeed(50),
      mode(0),
      autoplay(false),
      teamNumber(1),
      teamColor(0),
      locSimPathfile(""),
      skip(0) 
    { }
    
    void deserialize(const YAML::Node& node) {
      YAML_DESERIALIZE(node, simPlayers);
      YAML_DESERIALIZE(node, playSpeed);
      YAML_DESERIALIZE(node, mode);
      YAML_DESERIALIZE(node, autoplay);
      YAML_DESERIALIZE(node, teamNumber);
      YAML_DESERIALIZE(node, teamColor);
      YAML_DESERIALIZE(node, locSimPathfile);
      YAML_DESERIALIZE(node, skip);
      YAML_DESERIALIZE(node, seed);
    }

    void serialize(YAML::Emitter& emitter) const {
      YAML_SERIALIZE(emitter, simPlayers);
      YAML_SERIALIZE(emitter, playSpeed);
      YAML_SERIALIZE(emitter, mode);
      YAML_SERIALIZE(emitter, autoplay);
      YAML_SERIALIZE(emitter, teamNumber);
      YAML_SERIALIZE(emitter, teamColor);
      YAML_SERIALIZE(emitter, locSimPathfile);
      YAML_SERIALIZE(emitter, skip);
      YAML_SERIALIZE(emitter, seed);
    }

};

#endif
