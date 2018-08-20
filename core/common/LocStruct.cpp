#include <common/LocStruct.h>

constexpr float LocStruct::OrientConversion;

LocStruct::LocStruct() {
  robotSDX = robotSDY = sdOrient = 10.0f;
  ballCov = decltype(ballCov)::Identity() * 10'000;
  balls.fill(-10'000);
}

