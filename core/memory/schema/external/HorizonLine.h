#pragma once
#include <common/Serialization.h>
#include <vision/structures/HorizonLine.h>

DECLARE_EXTERNAL_SCHEMA(class HorizonLine {
  SCHEMA_FIELD(bool exists);
  SCHEMA_FIELD(float gradient);
  SCHEMA_FIELD(float offset);
});
