#include <common/annotations/SelectionType.h>

std::ostream& operator<<(std::ostream& os, SelectionType e) {
  return os << SelectionTypeMethods::getName(e);
}

YAML::Emitter& operator<<(YAML::Emitter& emitter, SelectionType e) {
  return emitter << SelectionTypeMethods::getName(e);
}

void operator>>(const YAML::Node& node, SelectionType& e) {
  std::string s;
  node >> s;
  e = SelectionTypeMethods::fromName(s);
}
