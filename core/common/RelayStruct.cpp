#include <common/TeamPacket.h>

RelayStruct& RelayStruct::operator=(const TeamPacket& other) {
  this->commData.packetsMissed = other.packetsMissed;
  this->bvrData = other.bvrData;
  this->locData = other.locData;
  this->sentTime = other.sentTime;
  return *this;
}
