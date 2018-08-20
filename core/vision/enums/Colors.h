#pragma once
#include <common/Enum.h>
#include <yaml-cpp/yaml.h>

/// @addtogroup vision
/// @{
ENUM(Color,
  c_UNDEFINED,
  c_FIELD_GREEN,
  c_WHITE,
  c_ORANGE,
  c_PINK,
  c_BLUE,
  c_YELLOW,
  c_ROBOT_WHITE
);
#define FLAG_GREEN (1 << c_GREEN)
#define FLAG_WHITE (1 << c_WHITE)
#define FLAG_ORANGE (1 << c_ORANGE)
#define FLAG_PINK (1 << c_PINK)
#define FLAG_BLUE (1 << c_BLUE)
#define FLAG_YELLOW (1 << c_YELLOW)

#define COLOR_NAME(c) ( \
    c == 0 ? "UNDEFINED" \
    : c == 1 ? "GREEN" \
    : c == 2 ? "WHITE" \
    : c == 3 ? "ORANGE" \
    : c == 4 ? "PINK" \
    : c == 5 ? "BLUE" \
    : c == 6 ? "YELLOW" \
    : c == 7 ? "ROBOT WHITE" \
    : "INVALID")

#define isInFlags(c,flags) (flags & (1 << c))
std::ostream& operator<<(std::ostream& os, Color color);
YAML::Emitter& operator<<(YAML::Emitter& os, Color color);
void operator>>(const YAML::Node& node, Color& color);
/// @}

