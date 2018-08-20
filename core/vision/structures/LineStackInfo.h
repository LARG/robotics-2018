#ifndef LINESTACKINFO_H
#define LINESTACKINFO_H

/// @ingroup vision
struct LineStackInfo {
  float diff;
  float doubleDiff;
  float diffStart;
  uint16_t posStart;
  uint16_t pointCount;
  uint16_t width;
};


#endif
