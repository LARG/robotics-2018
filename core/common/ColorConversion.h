#pragma once

#include <vision/VisionConstants.h>
#include <opencv2/core/core.hpp>
#include <common/ImageParams.h>

namespace color {

  struct Rgb {
    int r;
    int g;
    int b;
  };

  struct Yuv444 {
    int y;
    int u;
    int v;
  };

  struct Yuv422 {
    int y0;
    int u;
    int y1;
    int v;
  };

  inline int clip255(int x) {
    if (x > 255) return 255;
    if (x < 0) return 0;
    else return x;
  }

  inline Rgb toRgb(int r, int g, int b) {
    Rgb col;
    col.r = r;  
    col.g = g;
    col.b = b;
    return col;
  }

  inline Yuv444 rgbToYuv444 (int r, int g, int b) {
    Yuv444 yuv;
    yuv.y = ( ( (66 * r + 129 * g +  25 * b + 128) >> 8) +  16);
    yuv.u = ( ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128);
    yuv.v = ( ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128);
    return yuv;
  }

  inline Yuv444 rgbToYuv444 (const Rgb& rgb) {
    return rgbToYuv444(rgb.r, rgb.g, rgb.b);
  }

  inline Yuv422 rgbtoToYuv422 (const Rgb& rgb1, const Rgb& rgb2) {
    Yuv444 yuv1 = rgbToYuv444(rgb1);
    Yuv444 yuv2 = rgbToYuv444(rgb2);
    Yuv422 yuyv;
    yuyv.u = (int) ((yuv1.u + yuv2.u) / 2.0);
    yuyv.y0 = yuv1.y;
    yuyv.v = (int) ((yuv1.v + yuv2.v) / 2.0);
    yuyv.y1 = yuv2.y;
    return yuyv;
  }

  inline Rgb yuv444ToRgb (int y, int u, int v) {
    Rgb rgb;

    int C = y - 16;
    int D = u - 128;
    int E = v - 128;

    rgb.r = clip255(( 298 * C           + 409 * E + 128) >> 8);
    rgb.g = clip255(( 298 * C - 100 * D - 208 * E + 128) >> 8);
    rgb.b = clip255(( 298 * C + 516 * D           + 128) >> 8);
      
    return rgb;
  }

  inline Rgb yuv444ToRgb (const Yuv444& yuv) {
    return yuv444ToRgb(yuv.y, yuv.u, yuv.v);
  }

  inline void yuv422ToRgb(const Yuv422& yuyv, Rgb& rgb1, Rgb& rgb2) {
    rgb1 = yuv444ToRgb(yuyv.y0, yuyv.u, yuyv.v);
    rgb2 = yuv444ToRgb(yuyv.y1, yuyv.u, yuyv.v);
  }

  cv::Mat rawToMat(const unsigned char* imgraw, const ImageParams& iparams);
  cv::Mat rawToMatSubset(const unsigned char* imgraw, const ImageParams& params, int row, int col, int width, int height, int hstep, int vstep);
  cv::Mat rawToMatGraySubset(const unsigned char* imgraw, const ImageParams& params, int row, int col, int width, int height, int hstep, int vstep);

  void matToRaw(const cv::Mat& mat, unsigned char* imgraw, const ImageParams& iparams);
}
