#include "CommunicationModule.h"

#include <communications/RoboCupGameControlData.h>
#include <VisionCore.h>
#include <memory/LogWriter.h>
#include <memory/Lock.h>

#include <memory/FrameInfoBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/LocalizationBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/OpponentBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/CameraBlock.h>
#include <memory/BehaviorBlock.h>
#include <memory/AudioProcessingBlock.h>
#include <common/CameraParams.h>
#include <communications/PacketConverter.h>
#include <communications/CommInfo.h>
#include <communications/TCPConnection.h>
#include <math/Common.h>

#include <netdb.h>
#include <boost/lexical_cast.hpp>

#include <iostream>

#include <communications/Logging.h>


#define MAX_MEM_WRITE_SIZE MAX_STREAMING_MESSAGE_LEN

#define PACKETS_PER_SECOND 3
#define SECONDS_PER_PACKET (1.0f/PACKETS_PER_SECOND)
#define FRAMES_PER_PACKET (30/PACKETS_PER_SECOND)
#define PACKET_INVALID_DELAY 10
#define LOC_INVALID_DELAY 10

using namespace static_math;

#define print(str) std::cout << str << std::endl;

#include <mutex>
#include <condition_variable>
std::condition_variable STREAM_CV;
std::mutex STREAM_MUTEX;

bool* CommunicationModule::interpreter_restart_requested_(NULL);

CommunicationModule::CommunicationModule(VisionCore *core):
  teamUDP(NULL), toolUDP(NULL), gcDataUDP(NULL), gcReturnUDP(NULL),
  log_buffer_(NULL),
  connected_(false)
{
  core_ = core;
  vtime_ = 0;
  streaming_logger_ = std::make_unique<StreamLogWriter>();

  // Hack to get core to compile these methods since they're not used by core otherwise - JM 06/06/16 
  CommStruct s; s.setPacketsMissed(0, s.getPacketsMissed(0));
}

CommunicationModule::~CommunicationModule() {
  cleanupUDP();
  cleanupTCPServer();
  if (log_buffer_ != NULL)
    delete log_buffer_;
}

void CommunicationModule::specifyMemoryDependency() {
  requiresMemoryBlock("vision_frame_info");
  requiresMemoryBlock("game_state");
  requiresMemoryBlock("localization");
  requiresMemoryBlock("robot_state");
  requiresMemoryBlock("team_packets");
  requiresMemoryBlock("world_objects");
  requiresMemoryBlock("opponents");
  requiresMemoryBlock("vision_odometry");
  requiresMemoryBlock("camera_info");
  requiresMemoryBlock("behavior");
  requiresMemoryBlock("audio_processing");
  requiresMemoryBlock("vision_joint_commands");
}

void CommunicationModule::specifyMemoryBlocks() {
  getMemoryBlock(frame_info_,"vision_frame_info");
  getOrAddMemoryBlock(game_state_,"game_state");
  getOrAddMemoryBlock(robot_state_,"robot_state");
  getOrAddMemoryBlock(world_objects_,"world_objects");

  getOrAddMemoryBlock(localization_,"localization");
  getOrAddMemoryBlock(team_packets_,"team_packets");
  getOrAddMemoryBlock(opponents_,"opponents");
  getOrAddMemoryBlock(odometry_,"vision_odometry");
  getOrAddMemoryBlock(camera_,"camera_info");
  getOrAddMemoryBlock(behavior_,"behavior");
  getOrAddMemoryBlock(audio_processing_, "audio_processing");
  getOrAddMemoryBlock(joint_commands_,"vision_joint_commands");
}

void CommunicationModule::initSpecificModule() {
}

void CommunicationModule::cleanupUDP() {
  vector<UDPWrapper**> connections = { &teamUDP, &toolUDP, &gcDataUDP, &gcReturnUDP };
  for(auto c : connections) { 
    if(*c) delete *c;
    *c = NULL;
  }
}


void CommunicationModule::initUDP() {
  cleanupUDP();
  RobotConfig defaultConfig;
  if(CommInfo::TEAM_BROADCAST_IP == "")
    CommInfo::TEAM_BROADCAST_IP = defaultConfig.team_broadcast_ip;
  if(CommInfo::TEAM_UDP_PORT == 0)
    CommInfo::TEAM_UDP_PORT = defaultConfig.team_udp;
  teamUDP = new UDPWrapper(CommInfo::TEAM_UDP_PORT,true,CommInfo::TEAM_BROADCAST_IP);
  //bool broadcast = util::endswith(CommInfo::GAME_CONTROLLER_IP, ".255");
  toolUDP = new UDPWrapper(CommInfo::TOOL_UDP_PORT,false,CommInfo::TOOL_LISTEN_IP);
  //gcDataUDP = new UDPWrapper(GAMECONTROLLER_DATA_PORT,broadcast,CommInfo::GAME_CONTROLLER_IP);
  //gcReturnUDP = new UDPWrapper(GAMECONTROLLER_RETURN_PORT,broadcast,CommInfo::GAME_CONTROLLER_IP);
  teamUDP->startListenThread(&CommunicationModule::listenTeamUDP,this);
  toolUDP->startListenThread(&CommunicationModule::listenToolUDP,this);
  //gcDataUDP->startListenThread(&CommunicationModule::listenGameControllerUDP,this);
  printf("Initialized communication wrappers--\n\tBroadcast IP: %s\n\tTeam UDP: %i\n", 
    CommInfo::TEAM_BROADCAST_IP.c_str(),
    CommInfo::TEAM_UDP_PORT
  );
  connected_ = true;
  teamTimer_.start();
  gcTimer_.start();
}

void CommunicationModule::processFrame() {
  if(robot_state_->WO_SELF == WO_TEAM_LISTENER) return;
  else
    sendTeamUDP();
}

void CommunicationModule::sendGameControllerUDP() {
  RoboCupGameControlReturnData packet;
  packet.team = game_state_->gameContTeamNum;
  packet.player = robot_state_->WO_SELF;
  packet.message = GAMECONTROLLER_RETURN_MSG_ALIVE;
  bool res = gcReturnUDP->send(packet);
  //std::cout << "Sending GC message" << std::endl;
  if(!res) std::cerr << "Failed status message to Game Controller." << std::endl;
}

void CommunicationModule::listenTeamUDP() {
  SPLStandardMessage message;
  tlog(40, "Listening for packets.");
  bool res = this->teamUDP->recv(message, sizeof(message));
  tlog(40, "Received a UDP packet (frame %i), result: %i", this->frame_info_->frame_id, res);
  if(!res) return;
  if(this->robot_state_->ignore_comms_) return;
}

// send messages to teammates
void CommunicationModule::sendTeamUDP() {
}

void CommunicationModule::sendToolResponse(ToolPacket message) {
  toolUDP->sendToSender(message);
}

void CommunicationModule::sendToolRequest(ToolPacket message) {
  toolUDP->send(message);
}

void CommunicationModule::listenToolUDP() {
  // in use  P
  ToolPacket tp;
  bool res = this->toolUDP->recv(tp);
  if(!res) return;
  int prev_state = this->game_state_->state();

  switch(tp.message) {
    case ToolPacket::StateInitial: this->game_state_->setState(INITIAL); break;
    case ToolPacket::StateReady: this->game_state_->setState(READY); break;
    case ToolPacket::StateSet: this->game_state_->setState(SET); break;
    case ToolPacket::StatePlaying: this->game_state_->setState(PLAYING); break;
    case ToolPacket::StatePenalized: this->game_state_->setState(PENALISED); break;
    case ToolPacket::StateFinished: this->game_state_->setState(FINISHED); break;
    case ToolPacket::StateTesting: this->game_state_->setState(TESTING); break;
    case ToolPacket::StateTestOdometry: {
        this->game_state_->setState(TEST_ODOMETRY);
        this->behavior_->test_odom_fwd = tp.odom_command.x;
        this->behavior_->test_odom_side = tp.odom_command.y;
        this->behavior_->test_odom_turn = tp.odom_command.theta;
        this->behavior_->test_odom_walk_time = tp.odom_command.time;
        this->behavior_->test_odom_new = true;
      }
      break;
    case ToolPacket::StateCameraTop: this->game_state_->setState(TOP_CAM); break;
    case ToolPacket::StateCameraBottom: this->game_state_->setState(BOTTOM_CAM); break;
    case ToolPacket::LogSelect: {
        this->handleLoggingBlocksMessage(tp);
        this->core_->setLogSelections(tp);
      }
      break;
    case ToolPacket::LogBegin: {
        this->core_->enableLogging(tp.frames, tp.interval);
        printf("Logging %i frames, once per %2.2f second%s\n", tp.frames, tp.interval, tp.interval == 1 ? "" : "s");
      }
      break;
    case ToolPacket::LogEnd: this->core_->startDisableLogging(); break;
    case ToolPacket::RestartInterpreter:
      if(interpreter_restart_requested_) *interpreter_restart_requested_ = true;
      break;
    case ToolPacket::SetTopCameraParameters:
      this->camera_->set_top_params_ = true;
      this->camera_->comm_module_request_received_ = true;
      this->handleCameraParamsMessage(this->camera_->params_top_camera_, tp.data.data());
      cout << "CommunicationModule: Set top camera params" << endl;
      break;
    case ToolPacket::SetBottomCameraParameters:
      this->camera_->set_bottom_params_ = true;
      this->camera_->comm_module_request_received_ = true;
      this->handleCameraParamsMessage(this->camera_->params_bottom_camera_, tp.data.data());
      cout << "CommmunicationModule: Set bottom camera params" << endl;
      break;
    case ToolPacket::GetCameraParameters:
      this->camera_->get_top_params_ = true;
      this->camera_->get_bottom_params_ = true;
      this->camera_->comm_module_request_received_ = true;
      cout << "CommunicationModule: Read camera params" << endl;
      break;
    case ToolPacket::ResetCameraParameters: // reset camera
      std::cout << "CommunicationModule: Reset camera " << std::endl;
      this->camera_->reset_bottom_camera_ = true;
      this->camera_->reset_top_camera_ = true;
      this->camera_->comm_module_request_received_ = true;
      break;
    case ToolPacket::ManualControl: {
        this->game_state_->setState(MANUAL_CONTROL);
        this->behavior_->test_odom_fwd = tp.odom_command.x;
        this->behavior_->test_odom_side = tp.odom_command.y;
        this->behavior_->test_odom_turn = tp.odom_command.theta;
        this->behavior_->test_stance = tp.odom_command.stance;
        this->behavior_->test_odom_walk_time = 10.0f;
        this->behavior_->test_odom_new = true;
      }
      break;
    case ToolPacket::RunBehavior: {
        this->game_state_->setState(INITIAL);
        this->core_->interpreter_->runBehavior((char*)&tp.data);
      }
      break;
    case ToolPacket::SetStiffness: 
      for (int i = 0; i < NUM_JOINTS; i++){
        if (joint_commands_->stiffness_[i] > 0.9) {
          this->joint_commands_->setJointStiffness(i, tp.jointStiffness[i]);
          std::cout << tp.jointStiffness[i] << " ";
        }
      }
      std::cout << "Stiffness set" << std::endl;
      this->joint_commands_->send_stiffness_=true;
      this->joint_commands_->stiffness_time_=300;
      break;
  }

  if(prev_state != this->game_state_->state()) {
    printf("State changed from %s to %s\n", stateNames[prev_state].c_str(), stateNames[this->game_state_->state()].c_str());
  }
}

void CommunicationModule::handleCameraParamsMessage(CameraParams &params, char *msg) {

  std::vector<std::string> paramNames;
  std::vector<int> paramValues;

  int i = 0;
  std::string currentToken;

  bool tokenIsParamName = true;
  while (msg[i] != '|') {
    if (msg[i] == ' ') { // current token has ended
      if (tokenIsParamName) {
        paramNames.push_back(currentToken);
        tokenIsParamName = false;
      } else {
        paramValues.push_back(std::stoi(currentToken));
        tokenIsParamName = true;
      }
      currentToken.clear();
    } else {
      currentToken += msg[i];
    }
    i++;
  }

  for (unsigned i = 0; i < paramNames.size(); i++) {
    if (paramNames[i] == "AutoWhiteBalance") {
      params.kCameraAutoWhiteBalance = paramValues[i];
    } else if (paramNames[i] == "ExposureAuto") {
      params.kCameraExposureAuto = paramValues[i];
    } else if (paramNames[i] == "BacklightCompensation") {
      params.kCameraBacklightCompensation = paramValues[i];
    } else if (paramNames[i] == "Brightness") {
      params.kCameraBrightness = paramValues[i];
    } else if (paramNames[i] == "Contrast") {
      params.kCameraContrast = paramValues[i];
    } else if (paramNames[i] == "Saturation") {
      params.kCameraSaturation = paramValues[i];
    } else if (paramNames[i] == "Hue") {
      params.kCameraHue = paramValues[i];
    } else if (paramNames[i] == "Exposure") {
      params.kCameraExposure = paramValues[i];
    } else if (paramNames[i] == "Gain") {
      params.kCameraGain = paramValues[i];
    } else if (paramNames[i] == "Sharpness") {
      params.kCameraSharpness = paramValues[i];
    }
  }
}

void CommunicationModule::handleLoggingBlocksMessage(const ToolPacket& packet) {
  std::string block_name;
  bool log_block;
  int i = 0;
  const char* msg = packet.data.data();
  while (msg[i] != '|') {
    if (msg[i] == ' ') {
      i++;
      if (msg[i] == '0')
        log_block = false;
      else if (msg[i] == '1')
        log_block = true;
      else {
        std::cout << "bad logging info " << msg[i] << std::endl;
        return;
      }
      if (log_block) {
        std::cout << std::boolalpha << "setting logging of " << block_name << " to " << log_block << std::endl;
      }
      // special case for behavior trace (not a block)
      if (block_name.compare("behavior_trace") == 0){
        if (behavior_ != NULL)
          behavior_->log_behavior_trace_ = log_block;
      } else {
        memory_->setBlockLogging(block_name,log_block);
      }
      block_name.clear();
      i++;
      assert(msg[i] == ',');
    } else
      block_name += msg[i];
    i++;
  }
}

void CommunicationModule::listenGameControllerUDP() {
  RoboCupGameControlData gc;
  bool res = this->gcDataUDP->recv(gc);
  if(!res) return;
  if(this->robot_state_->ignore_comms_) return;

  return;

}

bool CommunicationModule::streaming() { return tcpserver_ != nullptr && tcpserver_->established(); }

void CommunicationModule::startTCPServer() {
  cleanupTCPServer();
  tcpserver_ = std::make_unique<TCPServer>(CommInfo::TOOL_TCP_PORT);
  tcpserver_->loop_server([this](auto result) {
    if(result == 0)
      printf("Started TCP Server, listening on port %i\n", tcpserver_->server_port());
    else {
      fprintf(stderr, "Error code %i starting TCP server: '%s'\n", result.value(), result.message().c_str());
    }
    this->cleanupStreamThread();
    stream_thread_ = std::make_unique<std::thread>(&CommunicationModule::stream, this);
  });
}

void CommunicationModule::cleanupTCPServer() {
  tcpserver_.reset();
  cleanupStreamThread();
}

void CommunicationModule::cleanupStreamThread() {
  if(stream_thread_ != nullptr) {
    stopping_stream_ = true;
    streaming_cv_.notify_one();
    stream_thread_->join();
    stream_thread_.reset();
    stopping_stream_ = false;
  }
}

void CommunicationModule::stream() {
  while(streaming()) {
    std::unique_lock<std::mutex> lock(streaming_mutex_);
    const auto& buffer = streaming_logger_->getBuffer();
    while(buffer.size == 0 && !stopping_stream_) {
      streaming_cv_.wait(lock, [&,this] { return buffer.size > 0 || stopping_stream_; });
      if(stopping_stream_) return;
    }
    tcpserver_->send_message(buffer);
    streaming_logger_->clearBuffer();
  }
}

void CommunicationModule::optionallyStream() {
  if(streaming()) {
    const StreamBuffer& buffer = streaming_logger_->getBuffer();
    if(buffer.size > 0) return;
    {
      std::lock_guard<std::mutex> lock(streaming_mutex_);
      streaming_logger_->writeMemory(*memory_);
    }
    streaming_cv_.notify_one();
  }
}

uint32_t CommunicationModule::getVirtualTime() {
  return vtime_;
}

void CommunicationModule::updateVirtualTime(uint32_t received) {
  vtime_ = max(vtime_, received);
}

void CommunicationModule::incrementVirtualTime() {
  vtime_++;
}
