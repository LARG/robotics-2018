#ifndef TEAMPACKETSBLOCK_
#define TEAMPACKETSBLOCK_

#include <common/TeamPacket.h>
#include <common/CoachPacket.h>
#include <memory/MemoryBlock.h>
#include <common/WorldObject.h>
#include <schema/gen/TeamPacketsBlock_generated.h>

// to be filled in by incoming team packets
#define TEAM_ARRAY_SIZE (WO_TEAM_LAST+1)
DECLARE_INTERNAL_SCHEMA(struct TeamPacketsBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(TeamPacketsBlock);
    TeamPacketsBlock();

    RelayStruct getPkt(unsigned int pkt) {
      return relayData[pkt];
    }
    
    RelayStruct* getPktPtr(unsigned int pkt) {
      return &(relayData[pkt]);
    }

    int getFrameReceived(unsigned int pkt) {
      return frameReceived[pkt];
    }

    int getMissedFromUs(unsigned int mate, unsigned int me){
      return 0;  // relayData[mate].commData.getPacketsMissed(me);
    }

    Point2D altBall(int self, int role, Point2D ball);

    CoachPacket cp;

    // Arrays of 6, so we can index from robot ID's 1-5. Index 0 not used.
    SCHEMA_FIELD(std::array<RelayStruct,TEAM_ARRAY_SIZE> relayData);
    SCHEMA_FIELD(std::array<OppStruct,TEAM_ARRAY_SIZE> oppData);

    SCHEMA_FIELD(std::array<int32_t,TEAM_ARRAY_SIZE> frameReceived);
    SCHEMA_FIELD(std::array<int32_t,TEAM_ARRAY_SIZE> ballUpdated);
    SCHEMA_FIELD(std::array<int8_t,TEAM_ARRAY_SIZE> oppUpdated);

    SCHEMA_FIELD(int sinceLastTeamPacketIn);
});

#endif 
