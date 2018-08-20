#ifndef SAMPLE_H
#define SAMPLE_H
#include <math/Geometry.h>
#include <common/RobotInfo.h>

/// @ingroup vision
struct Sample : Point2D {
    public:
      Camera::Type camera;
      float jointAngles[NUM_JOINTS];
      float torsoAngleX, torsoAngleY;
};
#endif
