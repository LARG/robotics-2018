#include <cstddef>
#include <memory/MemoryFrame.h>
#include <memory/MemoryCache.h>
#include <memory/WorldObjectBlock.h>
#include <memory/RobotStateBlock.h>

using std::move;
using std::string;
using std::unordered_set;

MemoryCache::MemoryCache() {
  clear();
}

MemoryCache::MemoryCache(MemoryFrame* memory) {
  fill(memory);
}

MemoryCache::MemoryCache(MemoryFrame& memory) {
  fill(memory);
}

void MemoryCache::clear() {
  world_object = nullptr;
  localization_mem = nullptr;
  team_packets = nullptr;
  frame_info = nullptr;
  robot_state = nullptr;
  game_state = nullptr;
  odometry = nullptr;
  joint = nullptr;
  behavior = nullptr;
  sonar = nullptr;
  delayed_localization = nullptr;
  body_model = nullptr;
  image = nullptr;
  robot_vision = nullptr;
  robot_info = nullptr;
  sensor = nullptr;
  opponent_mem = nullptr;
  behavior_params = nullptr;
  walk_request = nullptr;
  kick_request = nullptr;
  walk_params = nullptr;
  joint_command = nullptr;
  walk_info = nullptr;
  audio_processing = nullptr;
  sim_truth = nullptr;
  calibration = nullptr;

  memory = nullptr;
}

template <typename T>
void fill(MemoryFrame* memory, const unordered_set<string>& blocks, T*& ptr, const string& block) {
  if(!blocks.empty() && blocks.find(block) == blocks.end()) return;
  memory->getOrAddBlockByName(ptr, block);
}

void MemoryCache::fill(MemoryFrame& memory) {
  fill(&memory, {});
}

void MemoryCache::fill(MemoryFrame& memory, const unordered_set<string>& blocks) {
  fill(memory, move(blocks));
}

void MemoryCache::fill(MemoryFrame* memory) {
  fill(memory, {});
}

void MemoryCache::fill(MemoryFrame* memory, const unordered_set<string>& blocks) {
  ::fill(memory, blocks, world_object, "world_objects");
  ::fill(memory, blocks, localization_mem, "localization");
  ::fill(memory, blocks, team_packets, "team_packets");
  ::fill(memory, blocks, frame_info, "vision_frame_info");
  ::fill(memory, blocks, robot_state, "robot_state");
  ::fill(memory, blocks, game_state, "game_state");
  ::fill(memory, blocks, odometry, "vision_odometry");
  ::fill(memory, blocks, joint, "vision_joint_angles");
  ::fill(memory, blocks, behavior, "behavior");
  ::fill(memory, blocks, sonar, "vision_processed_sonar");
  ::fill(memory, blocks, delayed_localization, "delayed_localization");
  ::fill(memory, blocks, body_model, "body_model");
  ::fill(memory, blocks, image, "raw_image");
  ::fill(memory, blocks, camera, "camera_info");
  ::fill(memory, blocks, robot_vision, "robot_vision");
  ::fill(memory, blocks, robot_info, "robot_info");
  ::fill(memory, blocks, sensor, "processed_sensors");
  ::fill(memory, blocks, opponent_mem, "opponents");
  ::fill(memory, blocks, behavior_params, "behavior_params");
  ::fill(memory, blocks, walk_request, "vision_walk_request");
  ::fill(memory, blocks, kick_request, "vision_kick_request");
  ::fill(memory, blocks, walk_params, "vision_al_walk_param");
  ::fill(memory, blocks, joint_command, "vision_joint_commands");
  ::fill(memory, blocks, walk_info, "vision_walk_info");
  ::fill(memory, blocks, sim_truth, "sim_truth_data");
  ::fill(memory, blocks, audio_processing, "audio_processing");
  ::fill(memory, blocks, calibration, "calibration");

  this->memory = memory;
}

MemoryCache MemoryCache::read(MemoryFrame& memory) {
  return read(&memory);
}

MemoryCache MemoryCache::read(MemoryFrame* memory) {
  MemoryCache cache;
  memory->getBlockByName(cache.world_object, "world_objects", false);
  memory->getBlockByName(cache.localization_mem, "localization", false);
  memory->getBlockByName(cache.team_packets, "team_packets", false);
  memory->getBlockByName(cache.frame_info, "vision_frame_info", false);
  memory->getBlockByName(cache.robot_state, "robot_state", false);
  memory->getBlockByName(cache.game_state, "game_state", false);
  memory->getBlockByName(cache.odometry, "vision_odometry", false);
  memory->getBlockByName(cache.joint, "vision_joint_angles", false);
  memory->getBlockByName(cache.behavior, "behavior", false);
  memory->getBlockByName(cache.sonar, "vision_processed_sonar", false);
  memory->getBlockByName(cache.delayed_localization, "delayed_localization", false);
  memory->getBlockByName(cache.body_model, "body_model", false);
  memory->getBlockByName(cache.image, "raw_image", false);
  memory->getBlockByName(cache.camera, "camera_info", false);
  memory->getBlockByName(cache.robot_vision, "robot_vision", false);
  memory->getBlockByName(cache.robot_info, "robot_info", false);
  memory->getBlockByName(cache.sensor, "processed_sensors", false);
  memory->getBlockByName(cache.opponent_mem, "opponents", false);
  memory->getBlockByName(cache.behavior_params, "behavior_params", false);
  memory->getBlockByName(cache.walk_request, "vision_walk_request", false);
  memory->getBlockByName(cache.kick_request, "vision_kick_request", false);
  memory->getBlockByName(cache.walk_params, "vision_al_walk_param", false);
  memory->getBlockByName(cache.joint_command, "vision_joint_commands", false);
  memory->getBlockByName(cache.walk_info, "vision_walk_info", false);
  memory->getBlockByName(cache.sim_truth, "sim_truth_data", false);
  memory->getBlockByName(cache.audio_processing, "audio_processing", false);
  memory->getBlockByName(cache.calibration, "calibration", false);
  cache.memory = memory;
  return cache;
}

MemoryCache MemoryCache::create(int team, int player) {
  MemoryFrame* memory = new MemoryFrame(false, MemoryOwner::TOOL_MEM, team, player);
  MemoryCache cache(memory);
  cache.world_object->init();
  cache.robot_state->WO_SELF = cache.robot_state->global_index_ = player;
  cache.robot_state->team_ = team;
  return cache;
}
