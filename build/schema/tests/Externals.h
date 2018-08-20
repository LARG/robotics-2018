#pragma once
#include <common/Serialization.h>
#include <math/Pose2D.h>

DECLARE_EXTERNAL_SCHEMA(class Pose2D {
  SCHEMA_FIELD(float x);
  SCHEMA_FIELD(float y);
  SCHEMA_FIELD(float t);
  SCHEMA_SERIALIZATION({
    const auto& v = __source_object__;
    __builder__.add_t(v.t);
    __builder__.add_x(v.x);
    __builder__.add_y(v.y); 
  });
  SCHEMA_DESERIALIZATION({
    auto& v = __target_object__;
    v.t = data->t();
    v.x = data->x();
    v.y = data->y();
  });
});
