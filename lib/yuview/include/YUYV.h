#pragma once

#include <stdint.h>
#include <array>

namespace yuview {
  struct RGB {
    uint8_t r, g, b;
    RGB() = default;
    RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
  };

  struct YUV {
    uint8_t y, u, v;
    YUV() = default;
    YUV(uint8_t y, uint8_t u, uint8_t v) : y(y), u(u), v(v) { }
  };

  struct YUYV {
    YUYV() = default;
    YUYV(uint8_t y0, uint8_t u, uint8_t y1, uint8_t v) : y0(y0), u(u), y1(y1), v(v) { }
    uint8_t y0, u, y1, v;
    static RGB toRgb(int y, int u, int v);
    static YUV fromRgb(int r, int g, int b);
    std::array<RGB,2> toRgb() const;
    void fromRgb(std::array<RGB,2> rgb);
    void read(const uint8_t* buffer);
  };
}
