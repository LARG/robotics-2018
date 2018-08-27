#pragma once

#include <common/Enum.h>
#include <yaml-cpp/yaml.h>

ENUM_CLASS(SelectionType,
  Rectangle,
  Ellipse,
  Square,
  Polygon,
  Manual
);

std::ostream& operator<<(std::ostream& os, SelectionType type);
YAML::Emitter& operator<<(YAML::Emitter& emitter, SelectionType type);
void operator>>(const YAML::Node& node, SelectionType& type);
