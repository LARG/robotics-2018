#include "ButtonModule.h"

#include <memory/GameStateBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/SensorBlock.h>
#include <memory/SpeechBlock.h>
#include <memory/CameraBlock.h>

#define USE_LAB_BUTTONS

const float ButtonModule::MAX_CLICK_INTERVAL = 0.35f;
const float ButtonModule::MIN_CLICK_TIME = 0.01f; // accept anything
const float ButtonModule::MAX_CLICK_TIME = 0.75f;

#define USE_LAB_BUTTONS

ButtonModule::ButtonModule():
  center_(true),
  left_bumper_(false),
  right_bumper_(false),
  head_middle_(false)
{
}

void ButtonModule::specifyMemoryDependency() {
  requiresMemoryBlock("vision_frame_info");
  requiresMemoryBlock("game_state");
  requiresMemoryBlock("robot_state");
  requiresMemoryBlock("vision_sensors");
  requiresMemoryBlock("speech");
  requiresMemoryBlock("camera_info");
}

void ButtonModule::specifyMemoryBlocks() {
  getMemoryBlock(frame_info_, "vision_frame_info");
  getMemoryBlock(game_state_, "game_state");
  getMemoryBlock(robot_state_, "robot_state");
  getMemoryBlock(sensors_, "vision_sensors");
  getMemoryBlock(speech_, "speech");
  getMemoryBlock(camera_, "camera_info");
}

void ButtonModule::processButtons() {
  int state = game_state_->state();

  processButton(sensors_->values_[centerButton], 0, center_);
  processButton(sensors_->values_[bumperRL], sensors_->values_[bumperRR],right_bumper_);
  processButton(sensors_->values_[bumperLL], sensors_->values_[bumperLR],left_bumper_);
  processButton(sensors_->values_[headMiddle], 0, head_middle_);

  if (center_.new_result) {
    processCenterPresses();
    center_.reset();
  }

  if (head_middle_.new_result) {
    camera_->calibrate_white_balance_ = !camera_->calibrate_white_balance_;
    std::cout << "Head touched" << std::endl;
    head_middle_.reset();
  }

  if (right_bumper_.new_result) {
    if ((state==INITIAL) || (state == FINISHED)) {
      // Right foot changes team
      robot_state_->team_changed_ = true;
      if (robot_state_->team_ == TEAM_RED)
        robot_state_->team_ = TEAM_BLUE;
      else if (robot_state_->team_ == TEAM_BLUE)
        robot_state_->team_ = TEAM_RED;
      std::cout << "Team Changed to " << robot_state_->team_ << std::endl;
    }
    right_bumper_.reset();
  }
  
  if (left_bumper_.new_result) {
    if ((state==INITIAL) || (state == FINISHED)) {
      // left foot changes kick off
      if (game_state_->ourKickOff) {
        game_state_->ourKickOff = false;
      } else {
        game_state_->ourKickOff = true;
      }
      std::cout << "KickOff Changed to " << game_state_->ourKickOff << std::endl;
    }
    left_bumper_.reset();
  }
}

void ButtonModule::processCenterPresses() {
  int state = game_state_->state();

#ifdef USE_LAB_BUTTONS
  if ((center_.presses == 3) || (center_.presses == 4)) {
    // always transition to finished
    game_state_->setState(SET);
    speech_->say("set");
  } else if (center_.presses == 5) {
    game_state_->setState(FINISHED);
  } else if (center_.presses == 6) {
    sayIP();
  } else if (center_.presses == 7) {
    game_state_->setState(TESTING);
  } else if (center_.presses == 8 or center_.presses == 2) {
    game_state_->isPenaltyKick = (not game_state_->isPenaltyKick);
    std::cout << "Changed isPenaltyKick to ";
    std::cout << (game_state_->isPenaltyKick ? "true" : "false") << std::endl;
    speech_->say((game_state_->isPenaltyKick ? "Penalty" : "No Penalty"));
  } else {
    game_state_->lastStateChangeFromButton = true;
    if (state==PENALISED) {
      game_state_->setState(PLAYING);
      game_state_->lastTimeLeftPenalized = frame_info_->seconds_since_start;
    } else if (state == TESTING) {
      game_state_->setState(FINISHED);
    } else if (state == TEST_ODOMETRY) {
      game_state_->setState(FINISHED);
    } else if (state==READY) {
      game_state_->setState(SET);
    } else if (state==SET) {
      game_state_->setState(PLAYING);
    } else if (state==PLAYING) {
      game_state_->setState(PENALISED);
    } else if (state==FINISHED) {
      game_state_->setState(INITIAL);
    }
  }
#else
  if ((center_.presses == 3) || (center_.presses == 4)) {
    // always transition to finished
    robot_state_->ignore_comms_ = !robot_state_->ignore_comms_;
    if (robot_state_->ignore_comms_)
      speech_->say("OFF");
    else
      speech_->say("ON");
    //speech_->say("set");
    //game_state_->setState(SET);
  } else if (center_.presses == 5) {
    game_state_->setState(FINISHED);
  } else if (center_.presses == 6) {
    sayIP();
  } else {
    game_state_->lastStateChangeFromButton = true;
    if (state==PENALISED) {
      game_state_->setState(PLAYING);
      game_state_->lastTimeLeftPenalized = frame_info_->seconds_since_start;
    } else if (state == TESTING) {
      game_state_->setState(FINISHED);
    } else if (state == TEST_ODOMETRY) {
      game_state_->setState(FINISHED);
    } else if (state == FINISHED) {
      game_state_->setState(PENALISED);
    } else {
      game_state_->setState(PENALISED);
    }
  }
#endif
  if (state != game_state_->state())
    std::cout << "State Changed from " << stateNames[state] << " to " << stateNames[game_state_->state()] << std::endl;
}
  
void ButtonModule::processButton(float bump1, float bump2, ButtonInfo &button) {
  if ((bump1 > 0.5f) || (bump2 > 0.5f)) {
    if (button.start < 0)
      button.start = frame_info_->seconds_since_start;
    button.last = frame_info_->seconds_since_start;
  } else {
    float dt = frame_info_->seconds_since_start - button.start;
    if ((dt >= MIN_CLICK_TIME) && (dt <= MAX_CLICK_TIME)) {
      button.presses++;
    }
    button.start = -1;
  }

  if (button.presses > 0) {
    float max_interval = -1;
    if (button.allow_multiple)
      max_interval = MAX_CLICK_INTERVAL;
    if (frame_info_->seconds_since_start - button.last > max_interval)
      button.new_result = true;
  }
}
  
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
 
void ButtonModule::sayIP() {
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;
  char addressBuffer[INET_ADDRSTRLEN];
  std::string msg;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
      // is a valid IP4 Address
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      if (strcmp("eth0",ifa->ifa_name) == 0) {
        msg += "eeth , ";
        msg += addressBuffer;
        msg += ". ";
      } else if (strcmp("wlan0",ifa->ifa_name) == 0) {
        msg += "w lan , ";
        msg += addressBuffer;
        msg += ". ";
      }
    } else {
      continue;
    }
  }
  if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

  speech_->say(msg);
}
