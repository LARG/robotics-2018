#ifndef CORNERPOINT_H
#define CORNERPOINT_H

#include <vision/structures/FieldLine.h>

/// @ingroup vision
struct CornerPoint {
  float PosX, PosY;
  float DeSkewX, DeSkewY;
  bool Valid;
  FieldLine* Line[2];
  unsigned short CornerType, FieldObjectID;
  short Orientation, Direction;
};

#endif
