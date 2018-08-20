#ifndef TEAM_PACKET_H
#define TEAM_PACKET_H

#include <common/WorldObject.h>
#include <memory/BehaviorBlock.h>
#include <math/Geometry.h>
#include <Eigen/Core>
#include <communications/SPLStandardMessage.h>
#include <common/SetPlayInfo.h>
#include <common/PassInfo.h>
#include <common/OppStruct.h>
#include <common/RelayStruct.h>
#include <common/Serialization.h>
#include <schema/gen/TeamPacket_generated.h>

DECLARE_INTERNAL_SCHEMA(struct TeamPacket {
  SCHEMA_METHODS(TeamPacket);
  SCHEMA_FIELD(LocStruct locData);
  SCHEMA_FIELD(BehaviorStruct bvrData);
  SCHEMA_FIELD(uint32_t sentTime);
  SCHEMA_FIELD(std::array<uint8_t,WO_TEAM_LAST> packetsMissed);
  SCHEMA_FIELD(int8_t robotIP = -1);

  //int getPacketsMissed(int mate);
  //void setPacketsMissed(int mate, int missed);
});

#ifndef SWIG
static_assert(sizeof(TeamPacket) <= SPL_STANDARD_MESSAGE_DATA_SIZE, "ERROR: Team packets are larger than the allowed message size.");
#endif
#endif
