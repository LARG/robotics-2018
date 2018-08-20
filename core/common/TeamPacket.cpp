#include <common/TeamPacket.h>

#define PACKETS_MISSED_FACTOR 10
/*
int TeamPacket::getPacketsMissed(int mate) { 
  return packetsMissed[mate - 1] * PACKETS_MISSED_FACTOR; 
}

void TeamPacket::setPacketsMissed(int mate, int missed) {
  int compressed = missed / PACKETS_MISSED_FACTOR;
  compressed = std::min(compressed, 255);
  packetsMissed[mate - 1] = compressed;
}
*/
