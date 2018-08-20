#ifndef COORDINATES_H
#define COORDINATES_H

/// @ingroup vision
/* Represents coordinates in image space on the screen (x,y) */
struct Coordinates {
  int x;
  int y;
  Coordinates(int x, int y) : x(x), y(y) { }
  Coordinates() : x(0), y(0) { }
};

#endif
