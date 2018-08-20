#ifndef YAML_CONFIG_H
#define YAML_CONFIG_H

#define YAML_SERIALIZE(emitter,x) emitter << YAML::Key << #x << YAML::Value << x
#define YAML_S(x) YAML_SERIALIZE(emitter,x)
#define YAML_DESERIALIZE(node,x) node[#x] >> x
#define YAML_D(x) YAML_DESERIALIZE(node,x)

#include <yaml-cpp/yaml.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

class YamlConfig {
  public:
    virtual ~YamlConfig() = default;
    bool saveToFile(std::string file) const;
    bool loadFromFile(std::string file);
    inline bool save(std::string file) const { return saveToFile(file); }
    inline bool load(std::string file) { return loadFromFile(file); }
    std::string toString() const;

    friend YAML::Emitter& operator<<(YAML::Emitter& out, const YamlConfig& config);
    friend std::ostream& operator<<(std::ostream& os, const YamlConfig& config);
    friend void operator>>(const YAML::Node& node, YamlConfig& config);
  
  private:
    // These methods are private because they should not be called directly; the 
    // operator>> and operator<< methods are meant to be used for interacting with
    // a general YAML stream, while 'serialize' and 'deserialize' are meant to define 
    // the class-specific serialization and deserialization procedures.
    virtual void deserialize(const YAML::Node& node) = 0;
    virtual void serialize(YAML::Emitter& emitter) const = 0;
};

#endif
