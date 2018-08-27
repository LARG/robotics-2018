#include <common/CommStruct.h>

#define PACKETS_MISSED_FACTOR 10

int CommStruct::getPacketsMissed(int mate) { 
  return packetsMissed[mate - 1] * PACKETS_MISSED_FACTOR; 
}

void CommStruct::setPacketsMissed(int mate, int missed) {
  int compressed = missed / PACKETS_MISSED_FACTOR;
  compressed = std::min(compressed, 255);
  packetsMissed[mate - 1] = compressed;
}

