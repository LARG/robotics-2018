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

#include "StreamingMessage.h"

#include <netdb.h>
#include <boost/lexical_cast.hpp>

#include <iostream>

#define MAX_MEM_WRITE_SIZE MAX_STREAMING_MESSAGE_LEN

#define PACKETS_PER_SECOND 1
#define FRAMES_PER_PACKET (30/PACKETS_PER_SECOND)
#define PACKET_INVALID_DELAY 10
#define LOC_INVALID_DELAY 10

#define print(str) std::cout << str << std::endl;

#include <mutex>
#include <condition_variable>
std::condition_variable STREAM_CV;
std::mutex STREAM_MUTEX;

bool* CommunicationModule::interpreter_restart_requested_(NULL);

CommunicationModule::CommunicationModule(VisionCore *core):
  teamUDP(NULL), coachUDP(NULL), toolUDP(NULL), gameControllerUDP(NULL),
  io_service(),
  sock(io_service),
  stream_msg_(NULL),
  log_buffer_(NULL),
  stream_lock_(NULL)
{
  core_ = core;
  connected_ = false;
  streaming_logger_ = std::make_unique<StreamLogWriter>();
  tcp_connected_ = false;
  vtime_ = 0;
}

CommunicationModule::~CommunicationModule() {
  cleanUDP();

  if (stream_msg_ != NULL)
    delete stream_msg_;
  if (log_buffer_ != NULL)
    delete log_buffer_;
  if (stream_lock_ != NULL)
    delete stream_lock_;
}

void CommunicationModule::specifyMemoryDependency() {
  requiresMemoryBlock("vision_frame_info");
  print("vision_frame_info");
  requiresMemoryBlock("game_state");
  print("game_state");
  requiresMemoryBlock("localization");
  print("localization");
  requiresMemoryBlock("robot_state");
  print("robot_state");
  requiresMemoryBlock("team_packets");
  print("team_packets");
  requiresMemoryBlock("world_objects");
  print("world_objects");
  requiresMemoryBlock("opponents");
  print("opponents");
  requiresMemoryBlock("vision_odometry");
  print("vision_odometry");
  requiresMemoryBlock("camera_info");
  print("camera_info");
  requiresMemoryBlock("behavior");
  print("behavior");
  requiresMemoryBlock("audio_processing");
  print("audio_processing");
}

void CommunicationModule::specifyMemoryBlocks() {
  print("vision_frame_info");
  getMemoryBlock(frame_info_,"vision_frame_info");
  print("vis_frame done");
  getOrAddMemoryBlock(game_state_,"game_state");
  print("game_state");
  getOrAddMemoryBlock(robot_state_,"robot_state");
  print("robot_state");
  getOrAddMemoryBlock(world_objects_,"world_objects");
  print("world_objs");

  getOrAddMemoryBlock(localization_,"localization");
  print("localization");
  getOrAddMemoryBlock(team_packets_,"team_packets");
  print("team_packets");
  getOrAddMemoryBlock(opponents_,"opponents");
  print("opponents");
  getOrAddMemoryBlock(odometry_,"vision_odometry");
  print("vision_odom");
  getOrAddMemoryBlock(camera_,"camera_info");
  print("camera_info");
  getOrAddMemoryBlock(behavior_,"behavior");
  print("behavior");
  getOrAddMemoryBlock(audio_processing_, "audio_processing");
  print("audo_processing");
}

void CommunicationModule::initSpecificModule() {
  if ((stream_lock_ == NULL) && (core_->type_ != CORE_TOOLSIM)) {
    stream_lock_ = new Lock(Lock::getLockName(memory_,"STREAMLOCK"),true);
  }
}

void CommunicationModule::cleanUDP() {
  vector<UDPWrapper**> connections = { &teamUDP, &coachUDP, &toolUDP, &gameControllerUDP };
  for(auto c : connections) {
    if(*c) delete *c;
    *c = NULL;
  }
}


void CommunicationModule::initUDP() {
  cleanUDP();
  RobotConfig defaultConfig;
  if(CommInfo::TEAM_BROADCAST_IP == "")
    CommInfo::TEAM_BROADCAST_IP = defaultConfig.team_broadcast_ip;
  if(CommInfo::TEAM_UDP_PORT == 0)
    CommInfo::TEAM_UDP_PORT = defaultConfig.team_udp;
  teamUDP = new UDPWrapper(CommInfo::TEAM_UDP_PORT,true,CommInfo::TEAM_BROADCAST_IP.c_str());
  if(robot_state_->WO_SELF == WO_TEAM_COACH)
    coachUDP = new UDPWrapper(SPL_COACH_MESSAGE_PORT,true,CommInfo::TEAM_BROADCAST_IP.c_str());
  toolUDP = new UDPWrapper(CommInfo::TOOL_UDP_PORT,false,CommInfo::TOOL_LISTEN_IP);
  gameControllerUDP = new UDPWrapper(GAMECONTROLLER_PORT,true,CommInfo::TEAM_BROADCAST_IP.c_str());
  teamUDP->startListenThread(CommunicationModule::listenTeamUDP,this);
  //coachUDP->startListenThread(CommunicationModule::listenCoachUDP,this);
  toolUDP->startListenThread(CommunicationModule::listenToolUDP,this);
  gameControllerUDP->startListenThread(&(CommunicationModule::listenGameControllerUDP),this);
  printf("Initialized communication wrappers--\n\tBroadcast IP: %s\n\tTeam UDP: %i\n", 
    CommInfo::TEAM_BROADCAST_IP.c_str(),
    CommInfo::TEAM_UDP_PORT
  );
  connected_ = true;
  coachTimer_.start();
}

void CommunicationModule::processFrame() {
  if(robot_state_->WO_SELF == WO_TEAM_COACH)
    sendCoachUDP();
  else
    sendTeamUDP();
}

void CommunicationModule::listenCoachUDP(void* arg ) {
  CommunicationModule* module = reinterpret_cast<CommunicationModule*>(arg);
  SPLCoachMessage message;
  bool res = module->coachUDP->recv(message);
  if(!res) return;
}

void CommunicationModule::sendCoachUDP() {
}

void CommunicationModule::sendGameControllerUDP() {
  RoboCupGameControlReturnData packet;
  packet.team = game_state_->gameContTeamNum;
  packet.player = robot_state_->WO_SELF;
  packet.message = GAMECONTROLLER_RETURN_MSG_ALIVE;
  bool res = gameControllerUDP->send(packet);
  if(!res) std::cerr << "Failed status message to Game Controller." << std::endl;
}

void CommunicationModule::listenTeamUDP(void* arg ) {
  CommunicationModule* module = reinterpret_cast<CommunicationModule*>(arg);
  SPLStandardMessage message;
  TeamPacket tp;
  bool res = module->teamUDP->recv(message);
  if(!res) return;
  if(module->robot_state_->ignore_comms_) return;
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

void CommunicationModule::listenToolUDP(void* arg ) {
  // in use  P
  CommunicationModule* module = reinterpret_cast<CommunicationModule*>(arg);
  ToolPacket tp;
  bool res = module->toolUDP->recv(tp);
  if(!res) return;
  int prev_state = module->game_state_->state();

  switch(tp.message) {
    case ToolPacket::StateInitial: module->game_state_->setState(INITIAL); break;
    case ToolPacket::StateReady: module->game_state_->setState(READY); break;
    case ToolPacket::StateSet: module->game_state_->setState(SET); break;
    case ToolPacket::StatePlaying: module->game_state_->setState(PLAYING); break;
    case ToolPacket::StatePenalized: module->game_state_->setState(PENALISED); break;
    case ToolPacket::StateFinished: module->game_state_->setState(FINISHED); break;
    case ToolPacket::StateTesting: module->game_state_->setState(TESTING); break;
    case ToolPacket::StateTestOdometry: {
        module->game_state_->setState(TEST_ODOMETRY);
        module->behavior_->test_odom_fwd = tp.odom_command.x;
        module->behavior_->test_odom_side = tp.odom_command.y;
        module->behavior_->test_odom_turn = tp.odom_command.theta;
        module->behavior_->test_odom_walk_time = tp.odom_command.time;
        module->behavior_->test_odom_new = true;
      }
      break;
    case ToolPacket::StateCameraTop: module->game_state_->setState(TOP_CAM); break;
    case ToolPacket::StateCameraBottom: module->game_state_->setState(BOTTOM_CAM); break;
    case ToolPacket::LogSelect: {
        module->handleLoggingBlocksMessage((char*)&tp.data);
      }
      break;
    case ToolPacket::LogBegin: {
        module->core_->enableLogging(tp.frames, tp.interval);
        printf("Logging %i frames, once per %2.2f second%s\n", tp.frames, tp.interval, tp.interval == 1 ? "" : "s");
      }
      break;
    case ToolPacket::LogEnd: module->core_->startDisableLogging(); break;
    case ToolPacket::StreamBegin: module->startTCP(); break;
    case ToolPacket::StreamEnd: {
        if(module->tcp_connected_) module->tcp_connected_ = false;
        else printf("TCP already disconnected.\n");
      }
      break;
    case ToolPacket::RestartInterpreter:
      if(interpreter_restart_requested_) *interpreter_restart_requested_ = true;
      break;
    case ToolPacket::SetTopCameraParameters:
      module->camera_->set_top_params_ = true;
      module->camera_->comm_module_request_received_ = true;
      module->handleCameraParamsMessage(module->camera_->params_top_camera_, (char*)&tp.data);
      cout << "CommunicationModule: Set top camera params" << endl;
      break;
    case ToolPacket::SetBottomCameraParameters:
      module->camera_->set_bottom_params_ = true;
      module->camera_->comm_module_request_received_ = true;
      module->handleCameraParamsMessage(module->camera_->params_bottom_camera_, (char*)&tp.data);
      cout << "CommmunicationModule: Set bottom camera params" << endl;
      break;
    case ToolPacket::GetCameraParameters:
      module->camera_->get_top_params_ = true;
      module->camera_->get_bottom_params_ = true;
      module->camera_->comm_module_request_received_ = true;
      cout << "CommunicationModule: Read camera params" << endl;
      break;
    case ToolPacket::ResetCameraParameters: // reset camera
      std::cout << "CommunicationModule: Reset camera " << std::endl;
      module->camera_->reset_bottom_camera_ = true;
      module->camera_->reset_top_camera_ = true;
      module->camera_->comm_module_request_received_ = true;
      break;
    case ToolPacket::ManualControl: {
        module->game_state_->setState(MANUAL_CONTROL);
        module->behavior_->test_odom_fwd = tp.odom_command.x;
        module->behavior_->test_odom_side = tp.odom_command.y;
        module->behavior_->test_odom_turn = tp.odom_command.theta;
        module->behavior_->test_stance = tp.odom_command.stance;
        module->behavior_->test_odom_walk_time = 10.0f;
        module->behavior_->test_odom_new = true;
      }
      break;
    case ToolPacket::RunBehavior: {
        module->game_state_->setState(INITIAL);
        module->core_->interpreter_->runBehavior((char*)&tp.data);
      }
      break;
  }

  if(prev_state != module->game_state_->state()) {
    printf("State changed from %s to %s\n", stateNames[prev_state].c_str(), stateNames[module->game_state_->state()].c_str());
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
        paramValues.push_back(boost::lexical_cast<int>(currentToken));
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

void CommunicationModule::handleLoggingBlocksMessage(char *msg) {
  std::string block_name;
  bool log_block;
  int i = 0;
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

/** This is the callback function used when we receive a UDP packet
    from GameController
    /param void* arg is a pointer to the call. We then call receive gameControllerUDP
*/
void CommunicationModule::listenGameControllerUDP(void* arg ) {
  CommunicationModule* module = reinterpret_cast<CommunicationModule*>(arg);
  RoboCupGameControlData gc;
  bool res = module->gameControllerUDP->recv(gc);
  if(!res) return;
  if(module->robot_state_->ignore_comms_) return;
}

void CommunicationModule::startTCP() {
  tcp_connected_ = false;

  tcp::endpoint endpt(toolUDP->senderAddress(),CommInfo::TOOL_TCP_PORT);

  std::cout << "streaming to " << toolUDP->senderAddress() << ":" << CommInfo::TOOL_TCP_PORT << "\n";

  boost::system::error_code err;
  sock.connect(endpt,err);
  if (err != 0) {
    std::cout << "Error creating tcp connection: " << err// << std::endl;
              << " (" << err.message() << ")" << std::endl;
    return;
  }
  tcp_connected_ = true;
  if (stream_msg_ == NULL)
    stream_msg_ = new StreamingMessage();

  pthread_create(&stream_thread_,NULL,&stream,this);
}

void* stream(void *arg) {
  CommunicationModule *module = reinterpret_cast<CommunicationModule*>(arg);
  while (module->tcp_connected_)
    module->sendTCP();
  std::cout << "stream thread exitting" << std::endl;
  return NULL;
}

void CommunicationModule::optionallyStream() {
  if (tcp_connected_) {
    prepareSendTCP();
  } else if (sock.is_open()) {
    std::cout << "disconnecting tcp" << std::endl;
    sock.close();
  }
}

void CommunicationModule::prepareSendTCP() {
  const StreamBuffer& buffer = streaming_logger_->getBuffer();
  if(buffer.size > 0) return;
  {
    std::lock_guard<std::mutex> lock(STREAM_MUTEX);
    streaming_logger_->writeMemory(*memory_);
  }
  STREAM_CV.notify_one();
}

void CommunicationModule::sendTCP() {
  std::unique_lock<std::mutex> lock(STREAM_MUTEX);
  const StreamBuffer& buffer = streaming_logger_->getBuffer();
  while(buffer.size == 0) {
    STREAM_CV.wait(lock);
  }
  if (!stream_msg_->sendMessage(sock, buffer.buffer, buffer.size)) {
    std::cout << "Problem sending tcp, disconnecting" << std::endl;
    tcp_connected_ = false;
  }
  streaming_logger_->clearBuffer();
  lock.unlock();
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
