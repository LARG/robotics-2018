#include <common/Camera.h>
#include <common/Util.h>

std::ostream& operator<<(std::ostream& os, Camera::Type camera) {
  os << Camera::c_str(camera);
  return os;
}

YAML::Emitter& operator<<(YAML::Emitter& emitter, Camera::Type camera) {
  return emitter << Camera::c_str(camera);
}

void operator>>(const YAML::Node& node, Camera::Type& camera) {
  std::string s;
  node >> s;
  if(s == "Top" || s == "TOP" || s == "0")
    camera = Camera::TOP;
  else if(s == "Bottom" || s == "BOTTOM" || s == "1")
    camera = Camera::BOTTOM;
  else throw std::runtime_error(util::ssprintf("Invalid camera type: %s", s));
}
