#ifndef VISIONPOINT_H
#define VISIONPOINT_H

#include <inttypes.h>

/// @ingroup vision

/* This stores information on color "runs" (populated during classifier->constructRuns())
    lbIndex is filled in later by blob formation*/

struct VisionPoint {
  uint16_t xi, xf, dx, yi, yf, dy;
  uint16_t lbIndex;
  bool isValid;
};
#endif
