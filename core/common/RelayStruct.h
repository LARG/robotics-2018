#ifndef RELAY_STRUCT_H
#define RELAY_STRUCT_H

#include <common/LocStruct.h>
#include <common/BehaviorStruct.h>
#include <common/CommStruct.h>
#include <schema/gen/RelayStruct_generated.h>

class TeamPacket;

DECLARE_INTERNAL_SCHEMA(struct RelayStruct {
  SCHEMA_METHODS(RelayStruct);
  SCHEMA_FIELD(LocStruct locData);
  SCHEMA_FIELD(BehaviorStruct bvrData);
  SCHEMA_FIELD(CommStruct commData);
  SCHEMA_FIELD(uint32_t sentTime);

  RelayStruct& operator=(const TeamPacket& other);
});
#endif
