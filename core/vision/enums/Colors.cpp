#include <vision/enums/Colors.h>
#include <algorithm>

std::ostream& operator<<(std::ostream& os, Color color) { 
  return os << getName(color);
}

YAML::Emitter& operator<<(YAML::Emitter& emitter, Color color) {
  return emitter << getName(color);
}
void operator>>(const YAML::Node& node, Color& color) {
  // Legacy support of numeric colors
  auto is_number = [](const auto& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
      return !std::isdigit(c); 
    }) == s.end();
  };
  std::string s;
  node >> s;
  if(is_number(s))
    color = static_cast<Color>(std::stoi(s));
  else
    color = fromName_Color(s);
}
