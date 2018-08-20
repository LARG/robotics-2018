#ifndef SET_PLAY_INFO_H
#define SET_PLAY_INFO_H

#include <common/Serialization.h>
#include <schema/gen/SetPlayInfo_generated.h>
#include <memory/BehaviorParamBlock.h>

DECLARE_INTERNAL_SCHEMA(struct SetPlayInfo {
  SCHEMA_METHODS(SetPlayInfo);
  SCHEMA_FIELD(int type);
  SCHEMA_FIELD(bool reversed);
  SCHEMA_FIELD(int targetPlayer);

  SetPlayInfo():
    type(SetPlay::none),
    reversed(false),
    targetPlayer(-1)
  {
  }
});
#endif
