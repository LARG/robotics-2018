#include <motion/KickModule.h>
#include <common/Keyframe.h>
#include <memory/FrameInfoBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/JointBlock.h>
#include <memory/SensorBlock.h>
#include <memory/KickRequestBlock.h>

#define JOINT_EPSILON (3.f * DEG_T_RAD)
#define DEBUG false
#define HACK

KickModule::KickModule() : state_(Finished), sequence_(NULL) { }

void KickModule::initSpecificModule() {
  #ifndef HACK
  // For some reason, small changes to the default file are causing the walk
  // to behave weird. Unclear what the root issue is but the fix right now is
  // to load the kick on demand. Since kicks load quickly and happen
  // relatively infrequently this should not add too much overhead to the kick
  // behavior.
  auto file = cache_.memory->data_path_ + "/kicks/default.yaml";
  sequence_ = new KeyframeSequence();
  printf("Loading kick sequence from '%s'...", file.c_str());
  fflush(stdout);
  if(sequence_->load(file)) {
    printf("success!\n");
  } else {
    printf("failed!\n");
    sequence_ = NULL;
  }
  #endif
  initial_ = NULL;
}

void KickModule::start() {
  printf("Starting kick sequence\n");
  initStiffness();
  state_ = Initial;
  cache_.kick_request->kick_running_ = true;
  keyframe_ = 0;
  frames_ = 0;
  auto file = cache_.memory->data_path_ + "/kicks/default.yaml";
  #ifdef HACK
  sequence_ = new KeyframeSequence();
  printf("Loading kick sequence from '%s'...", file.c_str());
  fflush(stdout);
  if(sequence_->load(file)) {
    printf("success!\n");
  } else {
    printf("failed!\n");
    sequence_ = NULL;
  }
  #endif
  initial_ = new Keyframe(cache_.joint->values_, 0);
}

void KickModule::finish() {
  printf("Finishing kick sequence\n");
  state_ = Finished;
  cache_.kick_request->kick_running_ = false;
  cache_.kick_request->kick_type_ == Kick::NO_KICK;
  if(initial_) delete initial_;
  initial_ = NULL;
  #ifdef HACK
  if(sequence_) delete sequence_;
  sequence_ = NULL;
  #endif
}

bool KickModule::finished() {
  return state_ == Finished;
}

void KickModule::specifyMemoryDependency() {
  requiresMemoryBlock("frame_info");
  requiresMemoryBlock("walk_request");
  requiresMemoryBlock("processed_joint_angles");
  requiresMemoryBlock("processed_joint_commands");
  requiresMemoryBlock("odometry");
  requiresMemoryBlock("processed_sensors");
  requiresMemoryBlock("body_model");
  requiresMemoryBlock("kick_request");
}

void KickModule::specifyMemoryBlocks() {
  cache_.memory = memory_;
  getMemoryBlock(cache_.frame_info,"frame_info");
  getMemoryBlock(cache_.walk_request,"walk_request");
  getMemoryBlock(cache_.joint,"processed_joint_angles");
  getMemoryBlock(cache_.joint_command,"processed_joint_commands");
  getMemoryBlock(cache_.odometry,"odometry");
  getMemoryBlock(cache_.sensor,"processed_sensors");
  getMemoryBlock(cache_.body_model,"body_model");
  getMemoryBlock(cache_.kick_request,"kick_request");
}

void KickModule::processFrame() {
  if(cache_.kick_request->kick_type_ == Kick::STRAIGHT) {
    if(state_ == Finished) start();
  }
  if(state_ == Initial || state_ == Running) {
    cache_.kick_request->kick_running_ = true;
    performKick();
  }
}


void KickModule::initStiffness() {
  std::cout << "init stiffness!" << std::endl;
  for (int i=0; i < NUM_JOINTS; i++)
    cache_.joint_command->stiffness_[i] = 1.0;
  cache_.joint_command->send_stiffness_ = true;
  cache_.joint_command->stiffness_time_ = 10;
}

void KickModule::performKick() {
  initStiffness();
  if(DEBUG) printf("performKick, state: %s, keyframe: %i, frames: %i\n", getName(state_), keyframe_, frames_);
  if(state_ == Finished) return;
  if(sequence_ == NULL) return;
  if(keyframe_ >= sequence_->keyframes.size()) {
    finish();
    return;
  }
  auto& keyframe = sequence_->keyframes[keyframe_];
  if(state_ == Initial) {
    if(frames_ >= keyframe.frames) {
      state_ = Running;
      frames_ = 0;
    } else {
      moveToInitial(keyframe, frames_);
    }
  }
  if(state_ == Running) {
    if(keyframe_ == sequence_->keyframes.size() - 1) {
      finish();
      return;
    }
    auto& next = sequence_->keyframes[keyframe_ + 1];
    if(frames_ >= next.frames) {
      keyframe_++;
      frames_ = 0;
      performKick();
      return;
    }
    moveBetweenKeyframes(keyframe, next, frames_);
  }
  frames_++;
}

bool KickModule::reachedKeyframe(const Keyframe& keyframe) {
  for(int i = 0; i < NUM_JOINTS; i++) {
    if(fabs(cache_.joint->values_[i] - keyframe.joints[i]) > JOINT_EPSILON) {
      return false;
    }
  }
  return true;
}

void KickModule::moveToInitial(const Keyframe& keyframe, int cframe) {
  if(initial_ == NULL) return;
  moveBetweenKeyframes(*initial_, keyframe, cframe);
}

void KickModule::moveBetweenKeyframes(const Keyframe& start, const Keyframe& finish, int cframe) {
  if(cframe == 0) {
    if(DEBUG) printf("moving between keyframes, time: %i, joints:\n", finish.frames * 10);
    for(int i = 0; i < finish.joints.size(); i++)
      if(DEBUG) printf("j[%i]:%2.2f,", i, finish.joints[i] * RAD_T_DEG);
    if(DEBUG) printf("\n");
    cache_.joint_command->setSendAllAngles(true, finish.frames * 10);
    cache_.joint_command->setPoseRad(finish.joints.data());
  }
}
