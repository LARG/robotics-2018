#include <YUYV.h>

namespace yuview {

#define CLIP(x) (x > 255 ? 255 : x < 0 ? 0 : x)

  // From: https://en.wikipedia.org/wiki/YUV#Y.27UV444_to_RGB888_conversion
  RGB YUYV::toRgb(int y, int u, int v) {
    RGB pixel;

    int C = y - 16;
    int D = u - 128;
    int E = v - 128;

    pixel.r =   CLIP((298 * C + 409 * E + 128) >> 8);
    pixel.g = CLIP((298 * C - 100 * D - 208 * E + 128) >> 8);
    pixel.b =  CLIP((298 * C + 516 * D + 128) >> 8);
    return pixel;
  }

  std::array<RGB,2> YUYV::toRgb() const {
    std::array<RGB,2> pixels;
    pixels[0] = this->toRgb(y0,u,v);
    pixels[1] = this->toRgb(y1,u,v);
    return pixels;
  }

  YUV YUYV::fromRgb(int r, int g, int b) {
    YUV yuv;
    yuv.y = ((66 * r + 129 * g +  25 * b + 128) >> 8) + 16;
    yuv.u = (( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
    yuv.v = (( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;
    return yuv;
  }

  void YUYV::fromRgb(std::array<RGB,2> rgb) {
    auto yuv0 = fromRgb(rgb[0].r, rgb[0].g, rgb[0].b);
    auto yuv1 = fromRgb(rgb[1].r, rgb[1].g, rgb[1].b);
    y0 = yuv0.y;
    y1 = yuv1.y;
    u = yuv0.u/2 + yuv1.u/2;
    v = yuv0.v/2 + yuv1.v/2;
  }

  void YUYV::read(const uint8_t* buffer) {
    y0 = buffer[0];
    u = buffer[1];
    y1 = buffer[2];
    v = buffer[3];
  }
}
