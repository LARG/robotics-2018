#include <common/YamlConfig.h>

std::string YamlConfig::toString() const {
  YAML::Emitter emitter;
  emitter << YAML::BeginDoc;
  emitter << YAML::BeginMap;
  serialize(emitter);
  emitter << YAML::EndMap;
  emitter << YAML::EndDoc;
  return emitter.c_str();
}
    
bool YamlConfig::saveToFile(std::string file) const {
  std::ofstream output(file.c_str());
  if(!output.good()) return false;
  YAML::Emitter emitter;
  emitter << YAML::BeginDoc;
  emitter << YAML::BeginMap;
  serialize(emitter);
  emitter << YAML::EndMap;
  emitter << YAML::EndDoc;
  output << emitter.c_str();
  return true;
}

bool YamlConfig::loadFromFile(std::string file){
  std::ifstream input(file.c_str());
  if(!input.good()) {
    printf("Config file %s doesn't exist, ignoring.\n", file.c_str());
    return false;
  }
  try {
    YAML::Parser parser(input);
    YAML::Node doc;
    parser.GetNextDocument(doc);
    deserialize(doc);
  }
  catch (YAML::Exception& e) {
    printf("Config file %s invalid, ignoring.\n", file.c_str());
    return false; 
  }
  return true;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const YamlConfig& config) {
  out << YAML::BeginMap;
  config.serialize(out);
  out << YAML::EndMap;
  return out;
}

std::ostream& operator<<(std::ostream& os, const YamlConfig& config) {
  return os << config.toString();
}

void operator>>(const YAML::Node& node, YamlConfig& config) {
  config.deserialize(node);
}
