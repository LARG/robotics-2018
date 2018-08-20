#ifndef PACKET_CONVERTER_H
#define PACKET_CONVERTER_H

#include <communications/SPLCoachMessage.h>
#include <communications/SPLStandardMessage.h>
#include <common/TeamPacket.h>
#include <common/CoachPacket.h>
#include <cstring>

typedef TeamPacket UTStandardMessage;
typedef CoachPacket UTCoachMessage;

/// @ingroup communications
class PacketConverter {
  public:
    static UTStandardMessage convert(SPLStandardMessage message);
    static SPLStandardMessage convert(UTStandardMessage message);
    static UTCoachMessage convert(SPLCoachMessage message);
    static SPLCoachMessage convert(UTCoachMessage message);
};

#endif

