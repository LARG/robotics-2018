#ifndef COMMUNICATION_MODULE_H
#define COMMUNICATION_MODULE_H

#include <Module.h>
#include <communications/UDPWrapper.h>
#include <boost/asio.hpp>
#include <common/ToolPacket.h>
#include <common/Profiling.h>
#include <common/CameraParams.h>

#include <mutex>
#include <condition_variable>

class VisionCore;
class LogWriter;
class StreamingMessage;
class TCPServer;

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
class TeamPacket;
class JointCommandBlock;

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
  void cleanupUDP();
  void stream();

  void processFrame();
  void optionallyStream();

  static bool *interpreter_restart_requested_;
  bool streaming();

 private:
  void sendTeamUDP();
  void sendCoachUDP();
  void tryUpdateRelayData(const TeamPacket& packet, int robotNumber);

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
  JointCommandBlock *joint_commands_;
    
  //Udp for robot team communication
  //ThreadedUDPSocket teamUDP;
  UDPWrapper* teamUDP;
  void listenTeamUDP();

  //Udp for coach communication
  UDPWrapper* coachUDP;
  void listenCoachUDP();

  //Udp for the tools commands to the robot
  //ThreadedUDPSocket toolUDP;
  UDPWrapper* toolUDP;
  void listenToolUDP();
  void handleCameraParamsMessage(CameraParams &params, char *msg);

  void handleLoggingBlocksMessage(const ToolPacket& tp);

  //Messages from game controller
  //ThreadedUDPSocket gameControllerUDP;
  UDPWrapper *gcDataUDP, *gcReturnUDP;
  void listenGameControllerUDP();
  void sendGameControllerUDP();

 public:
  void sendToolResponse(ToolPacket message);
  void sendToolRequest(ToolPacket message);

  uint32_t getVirtualTime();
  void updateVirtualTime(uint32_t received);
  void incrementVirtualTime();
  void startTCPServer();
private:
  void cleanupTCPServer();
  void cleanupStreamThread();
  std::condition_variable streaming_cv_;
  std::mutex streaming_mutex_;
  void prepareSendTCP();
  std::unique_ptr<LogWriter> streaming_logger_;
  char *log_buffer_;
  std::unique_ptr<std::thread> stream_thread_;
  uint32_t vtime_;
  Timer coachTimer_, teamTimer_, gcTimer_;
  std::unique_ptr<TCPServer> tcpserver_;
  bool connected_, stopping_stream_ = false;
};

#endif /* end of include guard: COMMUNICATIONS_MODULE */
