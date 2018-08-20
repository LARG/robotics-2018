#include <iostream>
#include <csignal>
#include <MotionCore.h>
#include <memory/Lock.h>

#include <sys/wait.h>
#include <memory/FrameInfoBlock.h>
#include <memory/WalkRequestBlock.h>

MotionCore *core = NULL;
Lock *lock = NULL;

bool checkVisionLive = false;
double vision_dead_time = 5.0;
double vision_start_time_delay = 25.0;

boost::interprocess::named_semaphore *sem;

void handleExit();
void handleSignal(int sig);
void restartVision(int argc, char* argv[]);

double getSystemTime();

int main(int argc, char* argv[]) {
  std::cout << "Running Shared Motion\n";
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

  //create the motion core
  core = new MotionCore(CORE_INIT,true,team_num,player_num);
  
  FrameInfoBlock *vision_frame_info;
  core->memory_.getOrAddBlockByName(vision_frame_info,"vision_frame_info",MemoryOwner::VISION);
  FrameInfoBlock *frame_info;
  core->memory_.getOrAddBlockByName(frame_info,"frame_info");
  WalkRequestBlock *walk_request;
  core->memory_.getOrAddBlockByName(walk_request,"sync_walk_request",MemoryOwner::SYNC);

  core->memory_.setBlockLogging("frame_info",true);
  //core->memory_.setBlockLogging("body_model",true);
  //core->memory_.setBlockLogging("graphable",true);
  //core->memory_.setBlockLogging("walk_engine",true);
  //core->memory_.setBlockLogging("walk_request",true);
  //core->memory_.setBlockLogging("processed_sensors",true);
  //core->memory_.setBlockLogging("processed_joint_angles",true);
  
  lock = new Lock(Lock::getLockName(&(core->memory_),LOCK_MOTION));
  core->memory_.motion_vision_lock_ = new Lock(Lock::getLockName(&(core->memory_),LOCK_MOTION_VISION),true);

  double vision_start_time = frame_info->seconds_since_start + vision_start_time_delay;
  
  while (true) {
    lock->lock();
    while (core->alreadyProcessedFrame())
      lock->wait();
    
    core->preProcess();

    //lock->unlock();

    core->receiveData(); // reads in walk and joint requests from vision

    core->processMotionFrame();

    //lock->lock();
    core->postProcess();
    lock->unlock();
    lock->notify_one();

    core->publishData(); // outputs joint angles and sensors

    if (checkVisionLive) {
      // make sure that vision is running
      if ((frame_info->seconds_since_start - vision_frame_info->seconds_since_start > vision_dead_time) && (frame_info->seconds_since_start > vision_start_time)) {
        std::cout << "VISION IS DEAD" << std::endl;
        // stand still
        walk_request->stand();
        // restart vision
        restartVision(argc,argv);
        vision_start_time = frame_info->seconds_since_start + vision_start_time_delay;
      }
    }
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
  std::cerr << "CLEANING UP MOTION" << std::endl;
  cleanLock(lock);
  if (core != NULL) {
    cleanLock(core->memory_.motion_vision_lock_);
    delete core;
  }
  std::cerr << "DONE CLEANING UP MOTION" << std::endl;
}

void handleSignal(int sig) {
  //handleExit();
  exit(0);
}

void restartVision(int argc, char* argv[]) {
  std::string path = argv[0];
  size_t ind;

  std::cout << "**************************" << std::endl;
  std::cout << "STARTING VISION" << std::endl;
  std::cout << "**************************" << std::endl;

  // WARNING: assuming labels are motion and vision
  // WARNING: losing other args
  ind = path.find("motion");
  while (ind != std::string::npos) {
    path.replace(ind,6,"vision");
    ind = path.find("motion");
  }
  pid_t p = fork();
  if (p == 0) {
    // kill vision
    execlp("bash","bash","-c","kill -9 $(top -n1 -b | grep vision | grep -v grep | awk '{ print $1 }')",NULL);
    return;
  }

  // wait for kill to finish
  int status;
  wait(&status);

  p = fork();
  
  if (p == 0) {
    //child 2
    sleep(1.0); // just to make sure everything is cleaned up from the previous one
    execl(path.c_str(),path.c_str(),NULL);
    return;
  }
  // parent - pass
  //int status;
  //wait(&status);
  //std::cout << "status: " << status << std::endl;
}
