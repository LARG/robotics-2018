#ifndef LOC_STRUCT_H
#define LOC_STRUCT_H

#include <common/Serialization.h>
#include <schema/gen/LocStruct_generated.h>
#include <math/Geometry.h>
#include <Eigen/Core>

DECLARE_INTERNAL_SCHEMA(struct LocStruct {
  SCHEMA_METHODS(LocStruct);
    
  Eigen::Matrix<float, 2, 2, Eigen::DontAlign> ballCov;
  SCHEMA_FIELD(float robotSDX);
  SCHEMA_FIELD(float robotSDY);
  SCHEMA_FIELD(float sdOrient);
  SCHEMA_FIELD(std::array<int16_t,6> balls);
  SCHEMA_FIELD(int16_t robotX = 0);
  SCHEMA_FIELD(int16_t robotY = 0);
  SCHEMA_FIELD(int8_t iorientation);

  static constexpr float OrientConversion = RAD_T_DEG / 2;
  inline float orientation() const { return iorientation / OrientConversion; }
  inline void setOrientation(const int8_t orient) { iorientation = orient*OrientConversion; }
  inline Point2D ballPos() const { return Point2D(balls[0],balls[1]); }
  inline void setBallPos(const Point2D& pos) { balls[0] = pos.x; balls[1] = pos.y; }
  inline Point2D altBall1() const { return Point2D(balls[2],balls[3]); }
  inline Point2D altBall2() const { return Point2D(balls[4],balls[5]); }
  inline Point2D robotPos() const { return Point2D(robotX, robotY); }

  LocStruct();
});
#endif
