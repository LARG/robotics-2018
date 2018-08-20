#ifndef COMM_INFO_H
#define COMM_INFO_H

#include <string>

/// @ingroup communications
class CommInfo {
  public:
    static int TEAM_UDP_PORT;
    static const int TOOL_UDP_PORT;
    static const int TOOL_TCP_PORT;

    static const char* TOOL_LISTEN_IP;
    static std::string TEAM_BROADCAST_IP;
};

#endif
