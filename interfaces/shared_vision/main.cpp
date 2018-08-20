#include <iostream>
#include <VisionCore.h>
//#include <memory/LogReader.h>
//#include <memory/SensorBlock.h>
//#include <memory/JointBlock.h>
//#include <memory/FrameInfoBlock.h>
//#include <memory/BodyModelBlock.h>
//#include <memory/JointCommandBlock.h>
//#include <common/RobotInfo.h>
#include <memory/Lock.h>
#include <csignal>
#include "args/ArgumentParser.h"

VisionCore *core = NULL;

void handleExit();
void handleSignal(int sig);

int main(int argc, char* argv[]) {
  std::cout << "Running Shared Vision\n";
  // setup some cleanup functions
  atexit(&handleExit);
  signal(SIGTERM,handleSignal);
  signal(SIGABRT,handleSignal);
  signal(SIGINT,handleSignal);
  
  int team_num = 0;
  int player_num = 1;
  if (argc >= 2) {
    team_num = atoi(argv[1]);
    if (argc >= 3) {
      player_num = atoi(argv[2]);
    }
  } 

  std::cout << "Using team_num: " << team_num << " player_num: " << player_num  << std::endl;
  if (player_num < 1 || player_num > 4){
    std::cout << "Invalid player num (1-4): " << player_num << std::endl;
    exit(-1);
  }
  if (team_num < 0 || team_num > 1){
    std::cout << "Invalid team num (0,1): " << team_num << std::endl;
    exit(-1);
  }

  //create the vision core
  auto args = ArgumentParser::Parse(argc, argv);
  PythonInterface::EnableOptimization = !args.python_debug;
  core = new VisionCore(CORE_INIT,true,team_num,player_num);

  core->memory_->vision_lock_ = new Lock(Lock::getLockName(core->memory_,LOCK_VISION));
  core->memory_->motion_vision_lock_ = new Lock(Lock::getLockName(core->memory_,LOCK_MOTION_VISION));

  while (true) {
    core->processVisionFrame();
  }

  return 0;
}

void cleanLock(Lock *&lock) {
  if (lock != NULL) {
    lock->notify_one();
    if (lock->owns())
      lock->unlock();
    delete lock;
  }
}

void handleExit() {
  std::cerr << "CLEANING UP VISION" << std::endl;
  if (core != NULL) {
    cleanLock(core->memory_->vision_lock_);
    cleanLock(core->memory_->motion_vision_lock_);
    delete core;
  }
  std::cerr << "DONE CLEANING UP VISION" << std::endl;
}

void handleSignal(int sig) {
  exit(0);
}
