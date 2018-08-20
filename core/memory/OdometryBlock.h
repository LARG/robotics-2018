#ifndef ODOMETRY_
#define ODOMETRY_

#include <memory/MemoryBlock.h>
#include <math/Pose2D.h>
#include <schema/gen/OdometryBlock_generated.h>

// all info needed by localization about robot motions
// includes kicking, walking, falling

struct Getup {
  enum GetupType {
    NONE,
    UNKNOWN,
    FRONT,
    BACK,
    NUM_GETUPS
  };
};

struct Fall {
  enum FallDir {
    NONE,
    UNKNOWN,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    NUM_FALL_DIRS
  };
};

DECLARE_INTERNAL_SCHEMA(struct OdometryBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(OdometryBlock);

    OdometryBlock() {
      header.version = 3;
      header.size = sizeof(OdometryBlock);

      displacement = Pose2D(0,0,0);
      
      walkDisabled = false;

      standing = true;
      didKick = false;

      getting_up_side_ = Getup::NONE;
      fall_direction_ = Fall::NONE;
    }

    void reset() {
      //std::cout << "Odo reset" << std::endl;
      displacement = Pose2D(0,0,0);
      didKick = false;
      getting_up_side_ = Getup::NONE;

      // fall dir is not set by motion, so dont do the resetting in sync
      //fall_direction_ = Fall::NONE;
    }

    // walking
    SCHEMA_FIELD(Pose2D displacement);
    SCHEMA_FIELD(bool standing);
    SCHEMA_FIELD(bool walkDisabled);

    // kicking
    SCHEMA_FIELD(bool didKick);
    SCHEMA_FIELD(float kickVelocity);
    SCHEMA_FIELD(float kickHeading);

    // falling
    SCHEMA_FIELD(Getup::GetupType getting_up_side_);
    SCHEMA_FIELD(Fall::FallDir fall_direction_);
});

#endif 
