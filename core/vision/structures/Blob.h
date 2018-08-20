#ifndef BLOB_H
#define BLOB_H

#include <vision/VisionConstants.h>
#include <vector>
#include <inttypes.h>

/// @ingroup vision
struct Blob {
  uint16_t xi, xf, dx, yi, yf, dy;
  uint16_t lpCount;
  std::vector<uint32_t> lpIndex;
  float diffStart;
  float diffEnd;
  float doubleDiff;
  uint16_t widthStart;
  uint16_t widthEnd;
  uint16_t avgX;
  uint16_t avgY;
  float avgWidth;
  float correctPixelRatio;
  bool invalid;

  // GOAL DETECTION
  int edgeSize;
  int edgeStrength;

  Blob() : lpIndex(MAX_BLOB_VISIONPOINTS, 0) { }
};

/// @ingroup vision
bool sortBlobAreaPredicate(Blob* left, Blob* right);


#endif
