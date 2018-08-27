#ifndef COMM_STRUCT_H
#define COMM_STRUCT_H

#include <common/Serialization.h>
#include <schema/gen/CommStruct_generated.h>
#include <common/WorldObject.h>

DECLARE_INTERNAL_SCHEMA(struct CommStruct {
  SCHEMA_METHODS(CommStruct);
  SCHEMA_FIELD(std::array<uint8_t,WO_TEAM_LAST> packetsMissed);

  int getPacketsMissed(int mate);
  void setPacketsMissed(int mate, int missed);

});
#endif
