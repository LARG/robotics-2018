#ifndef LOG_METADATA_H
#define LOG_METADATA_H

#include <map>
#include <common/YamlConfig.h>

class LogMetadata : public YamlConfig {
  public:
    unsigned int frames;
    std::vector<uint64_t> offsets;
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

#endif
    



