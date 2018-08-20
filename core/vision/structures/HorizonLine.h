#ifndef HORIZON_LINE_H
#define HORIZON_LINE_H

#include <vision/CameraMatrix.h>
#include <stdio.h>

/// @ingroup vision
struct HorizonLine {
  HorizonLine() { exists = false; }
  bool exists;
  float gradient;
  float offset;
  Coordinates left, right;
  static HorizonLine generate(const ImageParams& iparams, const CameraMatrix& camera, int distance);
  inline bool isAbovePoint(int imageX, int imageY) { return imageY > imageX * gradient + offset; }
};

#endif
