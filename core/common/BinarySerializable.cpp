#include <common/BinarySerializable.h>

void BinarySerializable::saveToFile(std::string file) const {
  std::ofstream output(file.c_str(), std::ios::out | std::ios::binary);
  serialize(output);
}

bool BinarySerializable::loadFromFile(std::string file){
  std::ifstream input(file.c_str(), std::ios::in | std::ios::binary);
  if(!input.good()) {
    printf("Config file %s doesn't exist, ignoring.\n", file.c_str());
    return false;
  }
  try {
    deserialize(input);
  }
  catch (std::exception& e) {
    printf("Config file %s invalid, ignoring.\n", file.c_str());
    return false; 
  }
  return true;
}
