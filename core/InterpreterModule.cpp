#include <InterpreterModule.h>
#include <VisionCore.h>
#include <communications/CommunicationModule.h>
#include <audio/AudioModule.h>

InterpreterModule::InterpreterModule(VisionCore* core) :
    is_ok_(true),
    restart_requested_(false),
    core_(core),
    joint_values_(NUM_JOINTS),
    joint_stiffness_(NUM_JOINTS),
    sensor_values_(NUM_SENSORS) {
  timer_.setInterval(30 * 5);
}

InterpreterModule::~InterpreterModule() {
}

void InterpreterModule::specifyMemoryDependency() {
  requiresMemoryBlock("behavior");
  requiresMemoryBlock("camera_info");
  requiresMemoryBlock("game_state");
  requiresMemoryBlock("robot_state");
  requiresMemoryBlock("vision_joint_angles");
  requiresMemoryBlock("vision_kick_request");
  requiresMemoryBlock("vision_odometry");
  requiresMemoryBlock("vision_sensors");
  requiresMemoryBlock("vision_walk_request");
  requiresMemoryBlock("vision_walk_response");
  requiresMemoryBlock("vision_frame_info");
  requiresMemoryBlock("vision_kick_params");
  requiresMemoryBlock("vision_walk_param");
  requiresMemoryBlock("vision_al_walk_param");
  requiresMemoryBlock("world_objects");
  requiresMemoryBlock("team_packets");
  requiresMemoryBlock("opponents");
  requiresMemoryBlock("behavior_params");
  requiresMemoryBlock("vision_joint_commands");
  requiresMemoryBlock("vision_processed_sonar");
  requiresMemoryBlock("vision_walk_info");
  requiresMemoryBlock("vision_body_model");
  requiresMemoryBlock("robot_info");
  requiresMemoryBlock("speech");
  requiresMemoryBlock("localization");
  requiresMemoryBlock("audio_processing");
  
  // Disabling these for the tool because they are huge and the allocations fail due to fragmentation, and this restricts the number of log frames we can run. - JM 4/21/15
#ifndef TOOL
  requiresMemoryBlock("robot_vision");
  requiresMemoryBlock("raw_image");
#endif
}

void InterpreterModule::specifyMemoryBlocks() {
  getOrAddMemoryBlock(behavior_,"behavior");
  getOrAddMemoryBlock(camera_block_,"camera_info");
  getOrAddMemoryBlock(game_state_,"game_state");
  getOrAddMemoryBlock(robot_state_,"robot_state");
  getOrAddMemoryBlock(joint_angles_,"vision_joint_angles");
  getOrAddMemoryBlock(kick_request_,"vision_kick_request");
  getOrAddMemoryBlock(odometry_,"vision_odometry");
  getOrAddMemoryBlock(sensors_,"vision_sensors");
  getOrAddMemoryBlock(vision_frame_info_,"vision_frame_info");
  getOrAddMemoryBlock(walk_request_,"vision_walk_request");
  getOrAddMemoryBlock(walk_response_,"vision_walk_response");
  getOrAddMemoryBlock(world_objects_,"world_objects");
  getOrAddMemoryBlock(team_packets_,"team_packets");
  getOrAddMemoryBlock(opponents_,"opponents");
  getOrAddMemoryBlock(behavior_params_,"behavior_params");
  getOrAddMemoryBlock(joint_commands_,"vision_joint_commands");
  getOrAddMemoryBlock(vision_processed_sonar_,"vision_processed_sonar");

  getOrAddMemoryBlock(kick_params_,"vision_kick_params");
  getOrAddMemoryBlock(walk_param_,"vision_walk_param");
  getOrAddMemoryBlock(al_walk_param_,"vision_al_walk_param");
  getOrAddMemoryBlock(walk_info_,"vision_walk_info");
  getOrAddMemoryBlock(body_model_,"vision_body_model");

  getOrAddMemoryBlock(robot_info_,"robot_info");
  getOrAddMemoryBlock(speech_,"speech");
  getOrAddMemoryBlock(localization_,"localization");
  getOrAddMemoryBlock(audio_processing_,"audio_processing");

  // Disabling these for the tool because they are huge and the allocations fail due to fragmentation, and this restricts the number of log frames we can run. - JM 4/21/15
#ifndef TOOL
  getOrAddMemoryBlock(robot_vision_,"robot_vision");
  getOrAddMemoryBlock(image_,"raw_image");
#endif
}

void InterpreterModule::updatePercepts() {
  std::copy(joint_angles_->values_.begin(), joint_angles_->values_.end(), joint_values_.begin());
  std::copy(joint_angles_->stiffness_.begin(), joint_angles_->stiffness_.end(), joint_stiffness_.begin());
  std::copy(sensors_->values_.begin(), sensors_->values_.end(), sensor_values_.begin());
}

void InterpreterModule::readConfig() {
  // Read configuration file and place appropriate values in memory
  if (memory_->core_type_ == CORE_ROBOT) {
    RobotConfig config;
    std::cout << "Reading config file: " << memory_->data_path_ + "config.yaml" << std::endl;
    config.loadFromFile(memory_->data_path_ + "config.yaml"); // If the load fails we'll just use defaults
    robot_state_->robot_id_ = config.robot_id;
    game_state_->gameContTeamNum = config.team;
    if(config.team_broadcast_ip != "")
      CommInfo::TEAM_BROADCAST_IP = config.team_broadcast_ip;
    if(config.team_udp != 0)
      CommInfo::TEAM_UDP_PORT = config.team_udp;
    robot_state_->WO_SELF = robot_state_->global_index_ = config.self;
    robot_state_->role_ = config.role;
    std::cout << "From config file, read robot id: " << robot_state_->robot_id_ << ", GC team: " << game_state_->gameContTeamNum << ", wo_self: " << robot_state_->WO_SELF << std::endl;
    robot_state_->manual_pose_ = Pose2D(config.orientation * DEG_T_RAD, config.posX, config.posY);
    robot_state_->manual_height_ = config.posZ;
    walk_request_->walk_type_ = fromName_WalkType(config.walk_type);
    if(core_->communications_) core_->communications_->initUDP();
  }
}

bool InterpreterModule::getBool(bool *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setBool(bool *arr, int ind, bool val) {
  arr[ind] = val;
}

float InterpreterModule::getFloat(float *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setFloat(float *arr, int ind, float val) {
  arr[ind] = val;
}

double InterpreterModule::getDouble(double *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setDouble(double *arr, int ind, double val) {
  arr[ind] = val;
}

int InterpreterModule::getInt(int *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setInt(int *arr, int ind, int val) {
  arr[ind] = val;
}

unsigned char InterpreterModule::getUchar(unsigned char *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setUchar(unsigned char *arr, int ind, unsigned char val) {
  arr[ind] = val;
}

std::string InterpreterModule::getString(std::string *arr, int ind) {
  return arr[ind];
}

void InterpreterModule::setPose2D(Pose2D *arr,int ind, Pose2D val) {
  arr[ind] = val;
}
  
Pose2D InterpreterModule::getPose2D(Pose2D *arr,int ind) {
  return arr[ind];
}

void InterpreterModule::setPose3D(Pose3D *arr,int ind, Pose3D val) {
  arr[ind] = val;
}
  
Pose3D InterpreterModule::getPose3D(Pose3D *arr,int ind) {
  return arr[ind];
}

Pose3D* InterpreterModule::getPose3DPtr(Pose3D *arr,int ind) {
  return &(arr[ind]);
}

void InterpreterModule::log(int level, std::string message) {
  textlogger->log(level, TextLogger::Type::Behavior, message.c_str());
}

