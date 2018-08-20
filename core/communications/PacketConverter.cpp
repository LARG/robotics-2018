#include <communications/PacketConverter.h>

UTStandardMessage PacketConverter::convert(SPLStandardMessage message) {
  UTStandardMessage output;
  static_assert(sizeof(UTStandardMessage) < SPL_STANDARD_MESSAGE_DATA_SIZE, "Team packets are too large!");
  if(message.numOfDataBytes != sizeof(UTStandardMessage))
    return output;
  memcpy(&output, &message.data, message.numOfDataBytes);
  return output;
}

SPLStandardMessage PacketConverter::convert(UTStandardMessage message) {
  SPLStandardMessage output;
  memcpy(&output.data, &message, sizeof(UTStandardMessage));
  output.numOfDataBytes = sizeof(UTStandardMessage);
  return output;
}

UTCoachMessage PacketConverter::convert(SPLCoachMessage message) {
  UTCoachMessage output;
  return output;
}

SPLCoachMessage PacketConverter::convert(UTCoachMessage message) {
  SPLCoachMessage output;
  return output;
}
