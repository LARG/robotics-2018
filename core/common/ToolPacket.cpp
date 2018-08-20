#include <common/ToolPacket.h>
#include <string.h>

const int ToolPacket::DATA_LENGTH = __TOOL_PACKET_DATA_LENGTH;
ToolPacket::ToolPacket() : ToolPacket(None) { }
ToolPacket::ToolPacket(MessageType message) : message(message) {
  frames = 0;
  interval = 0;
  odom_command.x = odom_command.y = odom_command.theta = odom_command.time = 0;
  requiredObjects.fill(false);
  jointStiffness.fill(0.0f);
  data.fill(0);
  hasRequiredObjects = false;
}

ToolPacket::ToolPacket(MessageType message, std::string s) : message(message) {
  strncpy((char*)&data, s.c_str(), DATA_LENGTH);
}
