#pragma once
#include <common/Serialization.h>
#include <math/Vector2.h>

DECLARE_EXTERNAL_SCHEMA(class Vector2<float> {
  SCHEMA_FIELD(float x);
  SCHEMA_FIELD(float y);
  SCHEMA_SERIALIZATION({
    schema::Vector2_float_Builder __builder__(*__serializer__);
    const auto& v = __source_object__;
    __builder__.add_x(v[0]); 
    __builder__.add_y(v[1]); 
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& v = __target_object__;
    v[0] = data->x();
    v[1] = data->y(); 
  });
});
