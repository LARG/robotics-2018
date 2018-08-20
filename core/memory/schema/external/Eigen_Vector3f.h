#pragma once
#include <common/Serialization.h>
#include <Eigen/Core>

DECLARE_EXTERNAL_SCHEMA(class Eigen::Vector3f {
  SCHEMA_FIELD(float x);
  SCHEMA_FIELD(float y);
  SCHEMA_FIELD(float z);
  SCHEMA_SERIALIZATION({
    schema::Eigen::Vector3fBuilder __builder__(*__serializer__);
    const auto& v = __source_object__; 
    __builder__.add_x(v[0]);
    __builder__.add_y(v[1]); 
    __builder__.add_z(v[2]);
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& v = __target_object__;
    v[0] = data->x();
    v[1] = data->y(); 
    v[2] = data->z();
  });
});
