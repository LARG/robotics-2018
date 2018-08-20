#pragma once
#include <common/Serialization.h>
#include <kinematics/TorsoMatrix.h>
#include <algorithm>

DECLARE_EXTERNAL_SCHEMA(class TorsoMatrix {
  SCHEMA_FIELD(std::vector<float> offset);
  SCHEMA_FIELD(bool isValid);
  SCHEMA_SERIALIZATION({
    const auto& tmat = __source_object__;
    auto offset_alloc = __serializer__->CreateVector(tmat.offset.data(), tmat.offset.size());
    schema::TorsoMatrixBuilder __builder__(*__serializer__);
    __builder__.add_offset(offset_alloc);
    __builder__.add_isValid(tmat.isValid);
    return __builder__.Finish();
  });
  SCHEMA_DESERIALIZATION({
    auto& tmat = __target_object__;
    ::std::copy(data->offset()->begin(), data->offset()->end(), tmat.offset.data());
    tmat.isValid = data->isValid();
  });
});
