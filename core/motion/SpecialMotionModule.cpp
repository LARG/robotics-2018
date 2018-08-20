#include "SpecialMotionModule.h"

#include <memory/FrameInfoBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/JointBlock.h>
#include <memory/SensorBlock.h>
#include <memory/KickRequestBlock.h>

//#define SLOW_DEBUG
  
std::vector<std::vector<SpecialMotion> > SpecialMotionModule::motionSeqList = std::vector<std::vector<SpecialMotion> >();

SpecialMotionModule::SpecialMotionModule():
  teamID("test"),
  currMotion(NUM_Motions),
  state(FINISHED)
{
}

void SpecialMotionModule::initSpecificModule() {
  if (motionSeqList.size() != 0)
    return;
  std::cout << "start loading motion files" << std::endl << std::flush;
  loadMotionFiles();
  std::cout<<"done  loading motion files"<<std::endl;
}

void SpecialMotionModule::startMotion(Motion m) {
  std::cout << "STARTING MOTION " << getName(m) << std::endl;
  currMotion = m;
  transitionToState(INITIAL);
}

void SpecialMotionModule::loadMotionFiles() {
#ifdef SLOW_DEBUG
  float speedFactor = 0.2;
#else
  float speedFactor = 1.0;
#endif
  float timeConv = 1.0 / (1000.0 * speedFactor); // convert from ms to s and possible slow down for debugging

  std::string path = memory_->data_path_ + "/mof/" + teamID + "/";
  for (Motion m = (Motion)0; m != NUM_Motions; m = (Motion)(m + 1)) {
    motionSeqList.push_back(std::vector<SpecialMotion>());
    if (m == Getup) // Getup is a fake motion, will turn into a specific getup
      continue;
    std::vector<SpecialMotion> &motion = motionSeqList[m];

    std::string filename = path + getName(m) + ".mof";
    std::cout << "  reading " << filename << std::endl;
    // add the start
    motion.push_back(SpecialMotion(Motions::null));
    // read the motion
    SpecialMotionParser::ParseMotionFile(filename,motion);
    // convert to our units: angles to radians, divide hardnesses by 100
    for (unsigned int i = 0; i < motion.size(); i++) {
      for (int j = 0; j < NUM_JOINTS; j++) {
        switch (motion[i].mType) {
          case Motions::Null:
            break;
          case Motions::Hardness:
            motion[i].jointMotions[j] /= 100.0;
            break;
          case Motions::JointAngle:
            if (!isVoidNum(motion[i].jointMotions[j]))
              motion[i].jointMotions[j] *= DEG_T_RAD;
            break;
        }
      }
    }
    // convert the times
    for (unsigned int i = 0; i < motion.size(); i++) {
      motion[i].time *= timeConv;
      if (i > 0)
        motion[i].time += motion[i-1].time; // cumulative time
    }
#ifdef SLOW_DEBUG
    // print out some info about the motions
    std::cout << "  motion length:" << motion.size() << std::endl;
    for(int i = 0; i < motion.size(); i++) {   
      std::cout << "    pose " << m << ": ";
      for(int j = 0; j < NUM_JOINTS; j++)
        std::cout << motion[i].jointMotions[j] << " ";
      std::cout << std::endl;
    }
    std::cout << "  times: ";
    for(int i = 0; i < motion.size(); i++)
      std::cout << motion[i].time << " ";
    std::cout << std::endl;
#endif
    //std::cout << "  total motion time: " << motion.back().time << std::endl;
  } // end loop over motions

}

void SpecialMotionModule::specifyMemoryDependency() {
  requiresMemoryBlock("frame_info");
  requiresMemoryBlock("walk_request");
  requiresMemoryBlock("processed_joint_angles");
  requiresMemoryBlock("processed_joint_commands");
  requiresMemoryBlock("odometry");
  requiresMemoryBlock("processed_sensors");
  requiresMemoryBlock("body_model");
  requiresMemoryBlock("kick_request");
}

void SpecialMotionModule::specifyMemoryBlocks() {
  getMemoryBlock(frame_info_,"frame_info");
  getMemoryBlock(walk_request_,"walk_request");
  getMemoryBlock(joint_angles_,"processed_joint_angles");
  getMemoryBlock(commands_,"processed_joint_commands");
  getMemoryBlock(odometry_,"odometry");
  getMemoryBlock(sensors_,"processed_sensors");
  getMemoryBlock(body_model_,"body_model");
  getMemoryBlock(kick_request_,"kick_request");
}

void SpecialMotionModule::processFrame() {
  //commands_->send_stiffness_ = false;

  //if ((state == FINISHED) && (kick_request_->kick_type_ != Kick::NO_KICK) && (kick_request_->kick_type_ != Kick::ABORT))
    //startMotion(kickDiagonalNao);

  if (processFrameChild())
    return;

  switch (state) {
    case INITIAL:
      transitionToState(EXECUTE);
      startMotionSequence();
      break;
    case EXECUTE:
      if (isMotionDoneExecuting()) {
        std::cout << "Special Motion " << getName(currMotion) << " complete" << std::endl;
        transitionToState(WAIT);
      } else {
        executeMotionSequence();
        //commands_->setHeadPan(0,0,false);
        //commands_->angles_[LShoulderPitch] = DEG_T_RAD * -116;
        //commands_->angles_[LShoulderRoll] = DEG_T_RAD * 8;
        //commands_->angles_[LElbowYaw] = DEG_T_RAD * 25;
        //commands_->angles_[LElbowRoll] = DEG_T_RAD * -53;
        //commands_->angles_[RShoulderPitch] = DEG_T_RAD * -116;
        //commands_->angles_[RShoulderRoll] = DEG_T_RAD * 8;
        //commands_->angles_[RElbowYaw] = DEG_T_RAD * 25;
        //commands_->angles_[RElbowRoll] = DEG_T_RAD * -53;
      }
      break;
    case WAIT:
      if (getTimeInState() > 0.5)
        transitionToState(FINISHED);
      break;
    case FINISHED:
      break;
    case NO_ESCAPE:
      // never leave, there is no escape - for testing purposes only
      break;
    default:
      std::cout << "Unhandled specialMotion state: " << getName(state) << std::endl;
  } // switch
}
  
void SpecialMotionModule::transitionToState(State nstate) {
  state = nstate;
  stateStartTime = frame_info_->seconds_since_start;
}

float SpecialMotionModule::getTimeInState() {
  return frame_info_->seconds_since_start - stateStartTime;
}

//basic function to call joint command
void SpecialMotionModule::processJointCommands(float time, float angles[NUM_JOINTS]) {
  commands_->setSendAllAngles(true,time);
  float maxMove = DEG_T_RAD * 20;

  for (int i = 0; i < NUM_JOINTS; i++) {
    commands_->stiffness_[i] = 1.0; 
    float delta = angles[i] - storedCommands[i];
    if (fabs(delta) > maxMove) {
      commands_->angles_[i] = storedCommands[i] + delta / fabs(delta) * maxMove;
      if (i != RHipYawPitch) {
        std::cout << "  cropping " << JointNames[i] << std::endl;
        std::cout << "    " << JointNames[i] << " " << RAD_T_DEG * storedCommands[i] << " -> " << RAD_T_DEG * commands_->angles_[i] << " instead of: " << RAD_T_DEG * angles[i] << std::endl;
      }
    } else {
      commands_->angles_[i] = angles[i];
    }
    storedCommands[i] = commands_->angles_[i];
  }
}
  
bool SpecialMotionModule::isValueValid(float val) {
  return (val > -3) && (val < 3);
}

bool SpecialMotionModule::isVoidNum(float val) {
  return fabs(val - VOID_NUM) < 1;
}

void SpecialMotionModule::processJointHardness(float time,float stiffness[NUM_JOINTS]) {
  if (time == 0.0)
    return;
  commands_->send_stiffness_ = true;
  commands_->stiffness_time_ = time;
  for (int i = 0; i < NUM_JOINTS; i++) {
    float s = stiffness[i];
    if ((s < 0) || (s > 1.0))
      s = 1.0;
    commands_->stiffness_[i] = s;
  }
}


void SpecialMotionModule::startMotionSequence() {
  // set stiffnesses
  commands_->send_stiffness_ = true;
  commands_->stiffness_time_ = 10;
  for (int i = 0; i < NUM_JOINTS; i++)
    commands_->stiffness_[i] = 1.0;
}

int SpecialMotionModule::getCurrentFrame() {
  double timeInState = getTimeInState();
  std::vector<SpecialMotion> &motion = motionSeqList[currMotion];
  int curr = -1;
  for (int i = 1; i < (int)motion.size(); i++) {  
    if (timeInState < motion[i].time) {
      curr = i;
      break;
    }
  }
  // make sure that our indices are valid
  if (curr < 1) {
    curr = 1;
  }
  return curr;
}

void SpecialMotionModule::executeMotionSequence() {
  double timeInState = getTimeInState();
  std::vector<SpecialMotion> &motion = motionSeqList[currMotion];
  if (timeInState < 0.015) {
    for (int i = 0; i < NUM_JOINTS; i++)
      storedAngles[i] = joint_angles_->values_[i];
    for (int i = 0; i < NUM_JOINTS; i++)
      storedCommands[i] = joint_angles_->values_[i];
  }

  // figure out which frames we're between
  int curr = getCurrentFrame();
  int prev = curr - 1;
  if (curr >= (int)motion.size()) {
    std::cout<<"Finished motion sequence!!!"<<std::endl;
    return;
  }
  // calc frac of frame
  float frameFrac = (timeInState - motion[prev].time) / (motion[curr].time - motion[prev].time) ;
  //interp= interp*(FAST_FACTOR-interp* (FAST_FACTOR-1));
//  std::cout << num_seq_plus << " " << index <<" Diff"<<diff<< std::endl;
#ifdef SLOW_DEBUG
  std::cout << motion.size() << " curr= "<< curr << ", frac= " << frameFrac << std::endl;
#endif
  static int lastCurr = -1;
  if (curr != lastCurr) {
    std::cout << "curr = " << curr << std::endl;
    lastCurr = curr;
  }

  switch(motion[curr].mType) {
    case Motions::Hardness:
      processJointHardness(1, motion[curr].jointMotions);  
      break;
    case Motions::JointAngle: {
      float angles[NUM_JOINTS];
      //std::cout << motion[prev].jointMotions[0] << "(" << isValueValid(motion[prev].jointMotions[0]) << ")" << " " << motion[curr].jointMotions[0] << "(" << isValueValid(motion[curr].jointMotions[0]) << ")" << std::endl;
      for (int i = 0; i < NUM_JOINTS; i++) {
        if (!isValueValid(motion[curr].jointMotions[i])) { 
          // if current is void use VOID to update, else the update*interp will results in sth nonVOID
          storedAngles[i] = joint_angles_->values_[i];
          angles[i] = storedAngles[i];
        } else if ((motion[prev].mType != Motions::JointAngle) || !isValueValid(motion[prev].jointMotions[i])) {
          // if prev is jointAngle or VOID use current angle to interp
          angles[i] = storedAngles[i] + frameFrac * (motion[curr].jointMotions[i] - storedAngles[i]);
          //std::cout << joint_angles_->values_[i] << " -> " << angles[i] << " (stored:" << storedAngles[i] << " dest: " << motion[curr].jointMotions[i] << ")" << std::endl;
        } else {
          //use motion sequence to interp, otherwise
          angles[i] = motion[prev].jointMotions[i] + frameFrac * (motion[curr].jointMotions[i] - motion[prev].jointMotions[i]);
        }
        //std::cout << joint_angles_->values_[i] << " -> " << angles[i] << " (" << storedAngles[i] << ")" << std::endl;
      }
      processJointCommands(1,angles);
      }
      break;
    default:
      std::cout << "Unhandled mType: " << motion[curr].mType << std::endl;
      break;
  }
}




