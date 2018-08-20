#pragma once
#include <common/Serialization.h>
#include <math/Pose3D.h>
#include <algorithm>

DECLARE_EXTERNAL_SCHEMA(class Pose3D {
  SCHEMA_FIELD(std::vector<float> data);
  SCHEMA_SERIALIZATION({
    const auto& p = __source_object__;
    auto data_alloc = __serializer__->CreateVector(p.data(), p.size());
    schema::Pose3DBuilder __builder__(*__serializer__);
    __builder__.add_data(data_alloc);
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& p = __target_object__;
    ::std::copy(data->data()->begin(), data->data()->end(), p.data());
  });
});
