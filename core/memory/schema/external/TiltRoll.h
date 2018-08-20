#pragma once
#include <common/Serialization.h>
#include <common/TiltRoll.h>

DECLARE_EXTERNAL_SCHEMA(class TiltRoll {
  SCHEMA_FIELD(float tilt_);
  SCHEMA_FIELD(float roll_);
});
