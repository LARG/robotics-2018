#ifndef SPHERICAL_POSITION_H
#define SPHERICAL_POSITION_H

/// @ingroup vision
struct SphericalPosition {
  float r;
  float theta;
  float phi;
  SphericalPosition(int r, int theta, int phi) : r(r), theta(theta), phi(phi) { }
  SphericalPosition() : r(0), theta(0), phi(0) { }
  SphericalPosition(const Position&);
};

#endif
