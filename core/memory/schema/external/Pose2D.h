#pragma once
#include <common/Serialization.h>
#include <math/Pose2D.h>

DECLARE_EXTERNAL_SCHEMA(class Pose2D {
  SCHEMA_FIELD(float x);
  SCHEMA_FIELD(float y);
  SCHEMA_FIELD(float t);
  SCHEMA_SERIALIZATION({
    schema::Pose2DBuilder __builder__(*__serializer__);
    const auto& v = __source_object__;
    __builder__.add_t(v.rotation);
    __builder__.add_x(v.translation.x);
    __builder__.add_y(v.translation.y); 
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& v = __target_object__;
    v.rotation = data->t();
    v.translation.x = data->x();
    v.translation.y = data->y();
  });
});
