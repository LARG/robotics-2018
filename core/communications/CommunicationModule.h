#ifndef COMMUNICATION_MODULE_H
#define COMMUNICATION_MODULE_H

#include <Module.h>
#include <communications/UDPWrapper.h>
#include <boost/asio.hpp>
#include <common/ToolPacket.h>
#include <common/Profiling.h>
#include <common/CameraParams.h>

using boost::asio::ip::tcp;

class VisionCore;
class LogWriter;
class StreamingMessage;
class Lock;

class RobotStateBlock;
class GameStateBlock;
class LocalizationBlock;
class TeamPacketsBlock;
class WorldObjectBlock;
class FrameInfoBlock;
class OpponentBlock;
class OdometryBlock;
class CameraBlock;
class BehaviorBlock;
class AudioProcessingBlock;

void* stream(void *arg);

/** @ingroup communications
 * Responsible for managing communications between the teammates,
 * game controller, tool, and coach.
 */
class CommunicationModule: public Module {
 public:
  CommunicationModule(VisionCore *core);
  ~CommunicationModule();

  void specifyMemoryDependency();
  void specifyMemoryBlocks();
  void initSpecificModule();
  void initUDP();
  void cleanUDP();

  void processFrame();
  void optionallyStream();

  static bool *interpreter_restart_requested_;

 private:
  void sendTeamUDP();
  void sendCoachUDP();

  VisionCore *core_;

  // memory blocks
  FrameInfoBlock *frame_info_;
  RobotStateBlock *robot_state_;
  GameStateBlock *game_state_;
  LocalizationBlock *localization_;
  TeamPacketsBlock *team_packets_;
  WorldObjectBlock *world_objects_;
  OpponentBlock *opponents_;
  OdometryBlock *odometry_;
  CameraBlock *camera_;
  BehaviorBlock *behavior_;
  AudioProcessingBlock *audio_processing_;

  //Udp for robot team communication
  //ThreadedUDPSocket teamUDP;
  UDPWrapper* teamUDP;
  static void listenTeamUDP( void * );

  //Udp for coach communication
  UDPWrapper* coachUDP;
  static void listenCoachUDP( void * );

  //Udp for the tools commands to the robot
  //ThreadedUDPSocket toolUDP;
  UDPWrapper* toolUDP;
  static void listenToolUDP( void * );
  void handleCameraParamsMessage(CameraParams &params, char *msg);

  void handleLoggingBlocksMessage(char *);

  //Messages from game controller
  //ThreadedUDPSocket gameControllerUDP;
  UDPWrapper* gameControllerUDP;
  static void listenGameControllerUDP( void * );
  void sendGameControllerUDP();

  // for streaming to tool
public:
  bool tcp_connected_;
  void sendTCP();
  void sendToolResponse(ToolPacket message);
  void sendToolRequest(ToolPacket message);

  uint32_t getVirtualTime();
  void updateVirtualTime(uint32_t received);
  void incrementVirtualTime();
private:
  boost::asio::io_service io_service;
  tcp::socket sock;
  void startTCP();
  void prepareSendTCP();
  std::unique_ptr<LogWriter> streaming_logger_;
  StreamingMessage *stream_msg_;
  StreamBuffer cbuffer_;
  char *log_buffer_;
  Lock *stream_lock_;
  pthread_t stream_thread_;
  bool connected_;
  uint32_t vtime_;
  Timer coachTimer_;
};

#endif /* end of include guard: COMMUNICATIONS_MODULE */
