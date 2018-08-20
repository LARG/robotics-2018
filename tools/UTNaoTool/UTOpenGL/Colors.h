#ifndef TOOL_COLORS_H
#define TOOL_COLORS_H

#include <vector>
#include <common/ColorSpaces.h>

class Colors {
  public:
    static RGB White, Gray, Black, Pink;
    static RGB Red, Orange, Yellow, Green, Blue, Indigo, Violet, Brown, Cyan, Magenta;
    static RGB LightRed, LightOrange, LightYellow, LightGreen, LightBlue, LightIndigo, LightViolet, LightBrown, LightCyan, LightMagenta;
    static std::vector<RGB> StandardColors, LightColors;
};

#endif
