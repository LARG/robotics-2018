#include "GetupModule.h"

#include <memory/FrameInfoBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/SensorBlock.h>
#include <memory/JointBlock.h>

#define FIRST_FRAME_GRADUAL

const float PREPARE_ARMS_TIME = 0.30;
const float STIFFNESS_OFF_TIME = 0.25; 
const float CROSS_TIME = 0.5;
const float PAUSE_BEFORE_RESTART_TIME = 0.75;

GetupModule::GetupModule():
  numCrosses(0),
  numRestarts(0)
{
}

void GetupModule::initGetup() {
  getUpSide = Getup::UNKNOWN;
  startMotion(Getup);
  numCrosses = 0;
  numRestarts = 0;
  //std::cout << "INIT GETUP" << std::endl;
}

bool GetupModule::areJointsHot() {
  for(int i = 0; i < NUM_JOINTS; i++) {
    if (sensors_->joint_temperatures_[i] > 70)
      return true;
  }
  return false;
}

void GetupModule::setBackGetup() {
  odometry_->getting_up_side_ = Getup::BACK;
  getUpSide = Getup::BACK;
  currMotion = standUpBackNao;
}

void GetupModule::setFrontGetup() {
  odometry_->getting_up_side_ = Getup::FRONT;
  getUpSide = Getup::FRONT;
  currMotion = standUpFrontNao;
}

bool GetupModule::processFrameChild() {
  //std::cout << frame_info_->frame_id << " " << getName(state) << std::endl;
  // set getting up odometry
  if (isGettingUp()){
    // which way are we getting up?
    odometry_->getting_up_side_ = getUpSide;
  } else {
    // no getup
    odometry_->getting_up_side_ = Getup::NONE;
    // set getup side back to unknown for the next getup
    getUpSide = Getup::UNKNOWN;
  }

  bool fallenCountHigh = (abs(walk_request_->roll_fallen_counter_) >= 50) || (abs(walk_request_->tilt_fallen_counter_) >= 50);

  switch (state) {
    case INITIAL:
      if (walk_request_->getup_from_keeper_dive_) {
        transitionToState(CROSS); // goalie needs cross
      } else {
        if (fallenCountHigh)
          transitionToState(STIFFNESS_ON);
        else
          transitionToState(PREPARE_ARMS);
      }
      break;
    case PREPARE_ARMS:
      if ((getTimeInState() > PREPARE_ARMS_TIME) || fallenCountHigh) { // if fallen count is high, it's too late to prepare arms
        transitionToState(STIFFNESS_OFF);
      } else {
        if (getTimeInState() < 0.015) {
          prepareArms();
        }
        // otherwise do nothing
      }
      break;
    case STIFFNESS_OFF:
      if ((getTimeInState() > STIFFNESS_OFF_TIME) || fallenCountHigh) { // if fallen count is high, it's too late to worry about the stiffness being off
        transitionToState(STIFFNESS_ON);
      } else {
        commands_->setAllStiffness(0.0,10);
      }
      break;
    case STIFFNESS_ON:
      if (getTimeInState() > 0.001) {
        //if (armsStuckBehindBack()) {
          //std::cout << "Trying to free arms" << std::endl;
          //transitionToState(FREE_ARMS);
        //} else {
          transitionToState(EXECUTE);
        //}
      } else {
        commands_->setAllStiffness(1.0,10);
      }
      break;
    case CROSS:
      if (getTimeInState() < CROSS_TIME)
        cross();
      else {
        transitionToState(EXECUTE);
        numCrosses++;
      }
      break;
    case EXECUTE:
      if (currMotion == Getup) {
        selectGetup();
        if (currMotion == Getup) // didn't choose
          return true; // don't do anything else
        // yay! we chose a getup
        std::cout << "Starting getup: " << getName(currMotion) << std::endl;
        stateStartTime = frame_info_->seconds_since_start; // reset the time
      }
      if (isMotionDoneExecuting()) {
        if ((abs(walk_request_->tilt_fallen_counter_) > 2) || (abs(walk_request_->roll_fallen_counter_) > 2)) {
          // still fallen
          selectGetup();
          transitionToState(STIFFNESS_ON);
        } else {
          transitionToState(STAND);
        }
      } else {
        if (getTimeInState() < 0.015)
          commands_->setAllStiffness(1.0,10);
        //if (shouldAbortGetup()) {  // KG: for now, never abort.  Consider adding it back in if needed, but will need to also update method for new getup states
        //  std::cout << "FAILED GETUP" << std::endl;
        //  numRestarts++;
        //  transitionToState(PAUSE_BEFORE_RESTART);
        //}
        executeMotionSequence();
      }
      break;
    case STAND:
      // do nothing, handled in MotionCore
      if (getTimeInState() > 0.5)
        transitionToState(FINISHED);
      break;
    case FREE_ARMS:
      currMotion = backFreeArms;
      if (isMotionDoneExecuting()) {
        currMotion = Getup;
        transitionToState(EXECUTE);
      } else {
        if (getTimeInState() < 0.015)
          commands_->setAllStiffness(1.0,10);
        executeMotionSequence();
      }
      break;
    case PAUSE_BEFORE_RESTART:
      if (getTimeInState() > PAUSE_BEFORE_RESTART_TIME) {
        transitionToState(STIFFNESS_ON);
      } else {
        commands_->setAllStiffness(0.0,10);
      }
      break;
    default:
      return false;
      break;
  }
  return true;
}

void GetupModule::selectGetup() {
  int tiltSideThreshold = 5;
  if (numCrosses > 0) // be more confident after a cross
    tiltSideThreshold = 2;
  // decide which getup to do
  if (walk_request_->tilt_fallen_counter_ < -tiltSideThreshold) {
    setBackGetup();
  } else if (walk_request_->tilt_fallen_counter_ > tiltSideThreshold) {
    setFrontGetup();
  } else if ((numCrosses == 0) && (walk_request_->roll_fallen_counter_ >= 2)) {
    transitionToState(CROSS);
  } else if (getTimeInState() < 0.2) {
    // do nothing, maybe we'll end up somewhere good
  } else {
    setBackGetup(); // default to back in the end
  }
}

void GetupModule::setJointFromOffset(float angles[], Joint joint, float offset) {
  //angles[joint] = joint_angles_->values_[joint] + offset;
  angles[joint] = offset;
}

void GetupModule::prepareArms() {
  float angles[NUM_JOINTS];
  for (int i = 0; i < NUM_JOINTS; i++) {
    angles[i] = joint_angles_->values_[i];
  }
  setJointFromOffset(angles,HeadPitch,DEG_T_RAD * -15);
  //setJointFromOffset(angles,LShoulderPitch,DEG_T_RAD * -46);
  //setJointFromOffset(angles,LShoulderRoll,DEG_T_RAD * 32);
  //setJointFromOffset(angles,LElbowYaw,DEG_T_RAD * 80);
  //setJointFromOffset(angles,LElbowRoll,DEG_T_RAD * -50);
  //setJointFromOffset(angles,RShoulderPitch,DEG_T_RAD * -46);
  //setJointFromOffset(angles,RShoulderRoll,DEG_T_RAD * 32);
  //setJointFromOffset(angles,RElbowYaw,DEG_T_RAD * 80);
  //setJointFromOffset(angles,RElbowRoll,DEG_T_RAD * -50);
  setJointFromOffset(angles,LShoulderPitch,DEG_T_RAD * -116);
  setJointFromOffset(angles,LShoulderRoll,DEG_T_RAD * 12);
  setJointFromOffset(angles,LElbowYaw,DEG_T_RAD * -85);
  setJointFromOffset(angles,LElbowRoll,DEG_T_RAD * 0);
  setJointFromOffset(angles,RShoulderPitch,DEG_T_RAD * -116);
  setJointFromOffset(angles,RShoulderRoll,DEG_T_RAD * 12);
  setJointFromOffset(angles,RElbowYaw,DEG_T_RAD * -85);
  setJointFromOffset(angles,RElbowRoll,DEG_T_RAD * 0);
  
  processJointCommands(0,angles);
  commands_->setAllStiffness(0.0,0);
  commands_->stiffness_[HeadPitch] = 1.0;
  commands_->stiffness_[LShoulderPitch] = 1.0;
  commands_->stiffness_[LShoulderRoll] = 1.0;
  commands_->stiffness_[LElbowYaw] = 1.0;
  commands_->stiffness_[LElbowRoll] = 1.0;
  commands_->stiffness_[RShoulderPitch] = 1.0;
  commands_->stiffness_[RShoulderRoll] = 1.0;
  commands_->stiffness_[RElbowYaw] = 1.0;
  commands_->stiffness_[RElbowRoll] = 1.0;
}

void GetupModule::cross() {
  if (getTimeInState() > 0.025)
    return;
  float angles[NUM_JOINTS];
  for (int i = 0; i < NUM_JOINTS; i++) {
    angles[i] = 0;
  }

  angles[HeadPitch] = DEG_T_RAD*30;
  angles[LShoulderRoll] = 1.5708;
  angles[RShoulderRoll] = 1.5708;
  angles[LShoulderPitch] = -1.5708;
  angles[RShoulderPitch] = -1.5708;
  // walk arms
  //angles[LShoulderPitch] = DEG_T_RAD * -96; // -116 //moving to make it farther outside the body
  //angles[LShoulderRoll] = DEG_T_RAD * 8;
  angles[LElbowYaw] = DEG_T_RAD * 0;//25;
  angles[LElbowRoll] = DEG_T_RAD * 0;//-53;
  //angles[RShoulderPitch] = DEG_T_RAD * -96; // -116 //moving to make it farther outside the body
  //angles[RShoulderRoll] = DEG_T_RAD * 8;
  angles[RElbowYaw] = DEG_T_RAD * 0;//25;
  angles[RElbowRoll] = DEG_T_RAD * 0;//-53;

  processJointCommands(100,angles);
  commands_->setAllStiffness(1.0,10);
}

bool GetupModule::armsStuckBehindBack() {
  // check if we're on our front
  if (walk_request_->tilt_fallen_counter_ > 0)
    return false;
  //std::cout << "left " << body_model_->rel_parts_[BodyPart::left_hand].translation << " | right " << body_model_->rel_parts_[BodyPart::right_hand].translation << std::endl;
  // right arm
  if ((body_model_->rel_parts_[BodyPart::right_hand].translation.y > -44)
   && (body_model_->rel_parts_[BodyPart::right_hand].translation.x < 0))
    return true;
  // left arm
  if ((body_model_->rel_parts_[BodyPart::left_hand].translation.y < 44)
   && (body_model_->rel_parts_[BodyPart::left_hand].translation.x < 0))
    return true;
  return false;
}

bool GetupModule::shouldAbortGetup() {
  if (numRestarts >= 2)
    return false;

  int frame = getCurrentFrame();
  int frontVerticalFrame = 8;
  int backVerticalFrame = 6;

  int verticalFrame = frontVerticalFrame;
  if (currMotion == standUpBackNao) {
    verticalFrame = backVerticalFrame;
  }
  
  if (frame < verticalFrame) {
    angleYBuffer.init();
  } else {
    angleYBuffer.add(sensors_->values_[angleY]);
  }

  //std::cout << angleYBuffer.getNumberOfEntries() << " " << RAD_T_DEG * angleYBuffer.getAverage() << std::endl;
  if ((angleYBuffer.getNumberOfEntries() == angleYBuffer.getMaxEntries()) && (fabs(angleYBuffer.getAverage()) > DEG_T_RAD * 75)) {
    return true;
  }
  return false;
}
