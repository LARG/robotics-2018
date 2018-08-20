#include <communications/CommInfo.h>
int CommInfo::TEAM_UDP_PORT = 0;
/// @ingroup communications
/// Port numbers for the UT NAO Tool
const int CommInfo::TOOL_UDP_PORT = 78790;
const int CommInfo::TOOL_TCP_PORT = 53666;

/// @ingroup communications
/// IP addresses for the team, tool, and game controller
const char* CommInfo::TOOL_LISTEN_IP = "127.0.0.1";
std::string CommInfo::TEAM_BROADCAST_IP = "";
