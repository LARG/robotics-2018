#pragma once

#include <iostream>
#include <yaml-cpp/yaml.h>

class Camera {
  public:
    enum Type {
      TOP = 0,
      BOTTOM = 1
    };
    friend std::ostream& operator<<(std::ostream& os, Camera::Type t);
    friend YAML::Emitter& operator<<(YAML::Emitter& os, Camera::Type camera);
    friend void operator>>(const YAML::Node& node, Camera::Type& camera);
    static std::vector<std::string> names() { return { "Top", "Bottom" }; }
    static inline constexpr const char* c_str(Type t) { return t == TOP ? "Top" : "Bottom"; }
    template<Camera::Type t>
    static inline constexpr std::string str() {
      constexpr const char* cs = c_str(t);
      std::string s(cs);
      return s;
    }
    static inline std::vector<Camera::Type> cameras() { return { TOP, BOTTOM }; }
};
