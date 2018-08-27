#include "SimulatedPlayer.h"

// core
#include <VisionCore.h>

#include <communications/CommunicationModule.h>
#include <opponents/OppModule.h>

// memory
#include <memory/BehaviorBlock.h>
#include <memory/CameraBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/JointBlock.h>
#include <memory/KickRequestBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/MemoryFrame.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/OpponentBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/ALWalkParamBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/JointBlock.h>
#include <memory/LocalizationBlock.h>
#include <memory/ProcessedSonarBlock.h>
#include <memory/WalkInfoBlock.h>
#include <memory/BehaviorParamBlock.h>

#include <localization/LocalizationModule.h>

#include <stdlib.h>
#include <InterpreterModule.h>

#define index_ cache_.robot_state->global_index_

bool SimulatedPlayer::DEBUGGING_POSITIONING = false;
bool SimulatedPlayer::ALLOW_FALLING = false;

SimulatedPlayer::SimulatedPlayer(int team, int self, bool lMode) : iparams_(Camera::TOP), rmsim_(self + (team == TEAM_BLUE ? 0 : WO_TEAM_LAST)) {
  team_ = team;
  self_ = self;
  penaltySeconds = 0;
  kickSeconds = 0;
  kickHitSeconds = 0;
  getupSeconds = 0;
  diving = Dive::NONE;
  walkToTarget = false;
  locMode = lMode;
  dt = 1.0/30.0;

  panStopTime = 0;
  panMoving = false;

  PRINT = false;

  // noise factors (vision, cache_.odometry, kicking)
  visionErrorFactor = 1.0;//0.0;
  missedObsFactor = 1.0;//0.0;
  odometryErrorFactor = 1.0;
  kickErrorFactor = 1.0;
  
  visionErrorFactor = 1.5;
  missedObsFactor = 1.5;

  // params of how long things take
  getupTimeLength = 12.0; // get up is 12 seconds
  kickFullTime = 2.1;   // kick takes 2.1 seconds
  kickHitTime = 1.4;      // 1.4 seconds from start of kick to kick movement
  maxFwdVel = 240.0;
  maxSideVel = 120.0;
  maxTurnVel = 130.0 * DEG_T_RAD;

  // init vision core
  auto ltype = LocalizationMethod::Default;
  core = new VisionCore(CORE_TOOLSIM,false,team_,self_, ltype);

  // new memory
  memory_ = core->memory_;

  updateMemoryBlocks();

  cache_.robot_state->role_ = self_;
  cache_.world_object->init(team_);
  cache_.game_state->ourKickOff = (team_ == TEAM_BLUE);
  cache_.robot_state->robot_id_ = -1;

  // set no opponents
  for (int i = 0; i < MAX_OPP_MODELS_IN_MEM; i++){
    cache_.opponent_mem->locModels[i].alpha = -1;
  }

  // turn on text logging
  core->textlog_->onlineMode() = true;
  core->interpreter_->restart();
}

SimulatedPlayer::~SimulatedPlayer(){
  delete core;
  // core will delete memory for us
  core = NULL;
  memory_ = NULL;
}

void SimulatedPlayer::initLocalization() {
  core->localization_->initFromWorld();
}

// start the simulation from the last real snapshot
void SimulatedPlayer::setMemory(MemoryFrame* memory){
  core->updateMemory(memory);
  if (core->interpreter_){
    cout << "updating interpreter memory" << endl;
    core->interpreter_->updateModuleMemory(memory);
  }
  if (core->communications_){
    core->communications_->updateModuleMemory(memory_);
  }
  core->interpreter_->start();
}

void SimulatedPlayer::updateMemoryBlocks(){

  absWalkVel = Pose2D(0,0,0);
  relWalkVel = Pose2D(0,0,0);

  cache_.fill(memory_);

  // set the interpreter pointers again in case there were modules that didn't exist until this last call
  cache_.frame_info->source = MEMORY_SIM;
  if (core->interpreter_){
    cout << "updating interpreter memory" << endl;
    core->interpreter_->updateModuleMemory(memory_);
  }
  if (core->communications_){
    core->communications_->updateModuleMemory(memory_);
  }
  core->interpreter_->start();
}

bool SimulatedPlayer::processFrame() {
  return processFrame(cache_.world_object, cache_.game_state);
}

bool SimulatedPlayer::processFrame(WorldObjectBlock* simulationMem, GameStateBlock* simulationState){
  updateBasicInputs(simulationMem, simulationState);
  og_.setObjectBlocks(simulationMem, cache_.world_object);
  og_.setInfoBlocks(cache_.frame_info, cache_.joint);
  og_.setModelBlocks(cache_.opponent_mem);
  og_.setPlayer(index_, team_);

  // run localization
  if (locMode){
    og_.generateAllObservations();
    core->localization_->processFrame();
    core->opponents_->processFrame();
  } else {
    og_.generateGroundTruthObservations();
    WorldObject* ball = &(cache_.world_object->objects_[WO_BALL]);
    WorldObject* robot = &(cache_.world_object->objects_[self_]);
    cache_.behavior->keeperRelBallPos = ball->loc.globalToRelative(robot->loc,robot->orientation);
    cache_.behavior->keeperRelBallVel = ball->absVel.globalToRelative(Point2D(0,0),robot->orientation);
  }

  // call cache_.behavior process frame
  core->interpreter_->processBehaviorFrame();
  core->communications_->processFrame();

  return updateOutputs(simulationMem);

}

void SimulatedPlayer::updateBasicInputs(WorldObjectBlock* simulationMem, GameStateBlock* simulationState){

  // clear text debug
  core->textlog_->textEntries().clear();
  
  rmsim_.setCaches(simulationMem, cache_);
  rmsim_.step();

  // update game state from simulation mem
  if (cache_.game_state->state() != PENALISED)
    cache_.game_state->setState(simulationState->state());
  cache_.game_state->secsRemaining = simulationState->secsRemaining;
  if (team_ == TEAM_BLUE) cache_.game_state->ourKickOff = simulationState->ourKickOff;
  else cache_.game_state->ourKickOff = !simulationState->ourKickOff;
  cache_.game_state->isPenaltyKick = simulationState->isPenaltyKick;

  if (team_ == TEAM_BLUE) {
    cache_.game_state->ourScore = simulationState->ourScore;
    cache_.game_state->opponentScore = simulationState->opponentScore;
  } else {
    cache_.game_state->opponentScore = simulationState->ourScore;
    cache_.game_state->ourScore = simulationState->opponentScore;
  } 

  if (cache_.game_state->state() != PENALISED && cache_.game_state->state() != PLAYING)
    penaltySeconds = -dt;

  // check if penalty is over
  if (penaltySeconds <= 0 && cache_.game_state->state() == PENALISED){
    cache_.game_state->setState(simulationState->state());
  }

  if (cache_.game_state->state() == PENALISED && simulationState->state() != PLAYING){
    cache_.game_state->setState(simulationState->state());
    penaltySeconds = -dt;
  }

  if (cache_.game_state->state() == PENALISED){
    setPenaltyPosition(simulationMem);
  }

  // change frame info
  cache_.frame_info->seconds_since_start += dt;
  cache_.frame_info->frame_id++;
  if (penaltySeconds > 0)
    penaltySeconds -= dt;
  if (kickSeconds > 0)
    kickSeconds -= dt;
  if (kickHitSeconds > 0)
    kickHitSeconds -= dt;
  if (getupSeconds > 0)
    getupSeconds -= dt;

  // check if kick is complete
  if (kickSeconds <= 0.0){
    cache_.kick_request->kick_running_ = false;
    cache_.kick_request->kick_type_ = Kick::NO_KICK;
    cache_.walk_request->perform_kick_ = false;
    kickSeconds = 0;
  }

  if (kickHitSeconds <= 0)
    kickHitSeconds = 0;


  if (cache_.odometry->getting_up_side_ != Getup::NONE)
    diving = Dive::NONE;

  // check if get up is complete
  if (getupSeconds <= 0 && cache_.odometry->getting_up_side_ != Getup::NONE){
    cache_.odometry->getting_up_side_ = Getup::NONE;

    WorldObject* truthRobot = &(simulationMem->objects_[index_]);


    // with some small prob... get up the wrong direction
    if (Random::inst().sampleB(0.05)){
      // pick a dir to add 90 deg
      int choice = Random::inst().sampleU(1,3);
      if (PRINT) cout << index_ << " getting up in random direction " << endl;
      if (choice == 1)
        truthRobot->orientation -= M_PI/2.0;
      else if (choice == 2)
        truthRobot->orientation += M_PI/2.0;
      
    // if rolled, turn 90 deg sideway upon getup
    } else if (fabs(cache_.sensor->values_[angleX]) > 0.5){
      // rolled right and front getup will face right
      // rolled left and back getup will face right
      if ((cache_.sensor->values_[angleX] > 0 && cache_.odometry->getting_up_side_ == Getup::FRONT) || (cache_.sensor->values_[angleX] < 0 && cache_.odometry->getting_up_side_ == Getup::BACK)){
        truthRobot->orientation -= M_PI/2.0;
      } else {
        truthRobot->orientation += M_PI/2.0;
      }
    }
    // if tilted, still mainly same direction

    // some angle noise upon getup
    float oerror = Random::inst().sampleU(-.5,.5);
    truthRobot->orientation += odometryErrorFactor*0.1*M_PI*oerror;

    truthRobot->orientation = normalizeAngle(truthRobot->orientation);

    // also move the robot up to 50 cm in each dir upon getting up
    float randDistX = Random::inst().sampleU(-.5,.5) * 1000;
    truthRobot->loc.x += randDistX;
    float randDistY = Random::inst().sampleU(-.5,.5) * 1000;
    truthRobot->loc.y += randDistY;
    

    cache_.sensor->values_[angleX] = 0;
    cache_.sensor->values_[angleY] = 0;

  }

  // update sonar observations - to see changes in walk parameters
  cache_.sonar->on_center_ = false;
  cache_.sonar->on_left_ = false;
  cache_.sonar->on_right_ = false;
  cache_.sonar->bump_left_ = false;
  cache_.sonar->bump_right_ = false;

  cache_.sonar->center_distance_ = 2550;
  cache_.sonar->left_distance_ = 2550;
  cache_.sonar->right_distance_ = 2550;

  // update current walk velocities
  cache_.walk_info->robot_velocity_ = absWalkVel;
  cache_.walk_info->robot_velocity_frac_ = relWalkVel;

  WorldObject* truthRobot = &(simulationMem->objects_[index_]);
 
  for (int i = WO_TEAM_FIRST; i <= WO_OPPONENT_LAST; i++) {
    // skip us
    if (i == index_) continue;
    WorldObject* wo = &(simulationMem->objects_[i]);

    float distance = truthRobot->loc.getDistanceTo(wo->loc) / 1000.0 - 0.1;
    float bearing = truthRobot->loc.getBearingTo(wo->loc, truthRobot->orientation);
    
    if (distance < 0.4 && distance > 0 && !DEBUGGING_POSITIONING) {

      // Center
      // Todd: lets say its nearly always center (since it is)
      if (bearing < M_PI / 3 && bearing > - M_PI / 3) {
        if (cache_.sonar->on_center_) {
          cache_.sonar->center_distance_ = min(cache_.sonar->center_distance_, distance);
        } else {
          cache_.sonar->center_distance_ = distance;
          cache_.sonar->on_center_ = true;
        }
      }
      // Right
      else if (bearing <= - M_PI / 6 && bearing > -5 * M_PI / 12) {
        if (cache_.sonar->on_right_) {
          cache_.sonar->right_distance_ = min(cache_.sonar->right_distance_, distance);
        } else {
          cache_.sonar->right_distance_ = distance;
          cache_.sonar->on_right_ = true;
        }
      }
      // Left
      else if (bearing >= M_PI / 6 && bearing < 5 * M_PI / 12) {
        if (cache_.sonar->on_left_) {
          cache_.sonar->left_distance_ = min(cache_.sonar->left_distance_, distance);
        } else {
          cache_.sonar->left_distance_ = distance;
          cache_.sonar->on_left_ = true;
        }
      }
    }

    // simulate bump sensor
    if (false && wo->distance < 350) {
      if (fabs(M_PI/2.0 - bearing) < DEG_T_RAD*30.0){
        // left bump
        cache_.sonar->bump_left_ = true;
      }
      if (fabs(-M_PI/2.0 - bearing) < DEG_T_RAD*30.0){
        // right bump
        cache_.sonar->bump_right_ = true;
      }
    }
  } // opponents

  for (int i = 0; i < 2; i++){
    cache_.joint->prevValues_[i] = cache_.joint->values_[i];
  }
}

bool SimulatedPlayer::updateOutputs(WorldObjectBlock* simulationMem){
  // our location
  WorldObject* robot = &(cache_.world_object->objects_[self_]);
  WorldObject* ball = &(cache_.world_object->objects_[WO_BALL]);

  // update walk info so walk vel ramp-ups work properly
  cache_.odometry->didKick = false;
  cache_.odometry->standing = true;

  if (ALLOW_FALLING && !DEBUGGING_POSITIONING && Random::inst().sampleB(1.0/3600.))
    setFallen();

  // start timer
  if ((cache_.kick_request->kick_type_ != Kick::NO_KICK || cache_.walk_request->perform_kick_) && kickSeconds <= 0.0){
    float factor = 1;
    // walk kicks are faster
    if (cache_.walk_request->perform_kick_){
      factor = 0.4;
    }
    kickSeconds = kickFullTime * factor;
    kickHitSeconds = kickHitTime * factor;
    cache_.kick_request->kick_running_ = true;
  }

  // kick abort
  if (cache_.kick_request->kick_type_ == Kick::ABORT){
    cache_.kick_request->kick_type_ = Kick::NO_KICK;
    kickSeconds = 0;
    kickHitSeconds = 0;
    cache_.kick_request->kick_running_ = false;
  }

  //printf("kick type: %s, perform kick? %i, kick seconds: %2.2f\n", kickTypeNames[cache_.kick_request->kick_type_].c_str(), cache_.walk_request->perform_kick_, kickHitSeconds);
  // kick gets executed kickHitTime seconds in
  if ((cache_.kick_request->kick_type_ != Kick::NO_KICK || cache_.walk_request->perform_kick_) && cache_.kick_request->kick_type_ != Kick::ABORT && kickHitSeconds <= 0.2 && kickHitSeconds > 0){

    kickHitSeconds = 0;
    
    // this stuff only happens if ball is at robot!
    float ballThreshX = 210, ballThreshY = 90;
    if(cache_.walk_request->perform_kick_) {
      ballThreshX = 500, ballThreshY = 300;
    }
    if (ball->relPos.x < ballThreshX && fabs(ball->relPos.x) < ballThreshY && ball->relPos.x > 0) {

      // assume ball velocity at heading
      float vel = cache_.kick_request->desired_distance_;
      float heading = cache_.kick_request->desired_angle_;

      // unless this was from walk
      if (cache_.walk_request->perform_kick_){
        vel = cache_.walk_request->kick_distance_;
        heading = cache_.walk_request->kick_heading_;
      }

      cache_.odometry->didKick = true;
      cache_.odometry->kickHeading = heading;
      cache_.odometry->kickVelocity = vel / 1.2;
      cache_.walk_request->walk_control_status_ = WALK_CONTROL_DONE;
      // Fail with some probability (and print failure)
      // otherwise update normally
      if (Random::inst().sampleB(0.05)) {
        std::cout << "[SimulatedPlayer::updateOutputs]: Kick failed!!!";
        std::cout << std::endl;
        heading = 0;
        vel = 0;
      } else {
        std::cout << "[SimulatedPlayer::updateOutputs]: Kick success";
        std::cout << std::endl;
        // add up to 10 degree error
        heading += kickErrorFactor*Random::inst().sampleU(-.5,.5)*20.0*DEG_T_RAD;
        // add up to 40 % vel error
        vel += Random::inst().sampleU(-.5,.5)*kickErrorFactor*0.8*vel;

        // never longer than 8000
        if (vel > 8000) vel = 8000;

        // never shorter than 10
        if (vel < 10) vel = 10;

      }

      cache_.kick_request->kick_type_ = Kick::NO_KICK;

      // update to simulation mem
      WorldObject* truthRobot = &(simulationMem->objects_[index_]);
      WorldObject* truthBall = &(simulationMem->objects_[WO_BALL]);

      Point2D absVel(vel, truthRobot->orientation + heading, POLAR);
      if (!DEBUGGING_POSITIONING) {
        truthBall->absVel = absVel;
        return true;
      }
    } else {
      //cout << " kick failed, ball dist: " << ball->distance << " rel: " << ball->relPos << endl;
      cache_.kick_request->kick_type_ = Kick::NO_KICK;

    }
  }

  // kick or getup makes head to go to 0
  if (cache_.kick_request->kick_running_ || cache_.odometry->getting_up_side_ != Getup::NONE || cache_.odometry->fall_direction_ != Fall::NONE){
    cache_.joint_command->head_yaw_angle_change_ = false;
    cache_.joint_command->angles_[HeadYaw] = 0;
    cache_.joint_command->send_head_yaw_angle_ = true;
    cache_.joint_command->head_yaw_angle_time_ = 200;
  }

  if (diving != cache_.behavior->keeperDiving && cache_.behavior->keeperDiving != Dive::NONE)
    if (PRINT) cout << index_ << " diving" << endl;


  // diving
  if (cache_.behavior->keeperDiving == Dive::RIGHT || (diving == Dive::RIGHT && cache_.sensor->values_[angleX] < M_PI/2.0)){
    diving = Dive::RIGHT;
    if (cache_.sensor->values_[angleX] < M_PI/2.0){
      cache_.sensor->values_[angleX] += M_PI/20.0;
    }
  } else if (cache_.behavior->keeperDiving == Dive::LEFT || (diving == Dive::LEFT && cache_.sensor->values_[angleX] > -M_PI/2.0)){
    diving = Dive::LEFT;
    if (cache_.sensor->values_[angleX] > -M_PI/2.0){
      cache_.sensor->values_[angleX] -= M_PI/20.0;
    }
  } else if (cache_.behavior->keeperDiving == Dive::NONE){
    diving = Dive::NONE;
  }

  // start getting up
  if (cache_.walk_request->motion_ == WalkRequestBlock::FALLING && cache_.odometry->getting_up_side_ == Getup::NONE){
    if (getupSeconds <= 0) getupSeconds = getupTimeLength;
    cache_.behavior->keeperDiving = Dive::NONE;
    cache_.odometry->getting_up_side_ = Getup::UNKNOWN;
  }

  // continue get up, finish cross and choose a side to get up from
  if (getupSeconds < (getupTimeLength-0.5) && getupSeconds > (getupTimeLength-1.0)){
    if (cache_.sensor->values_[angleY] > 0)
      cache_.odometry->getting_up_side_ = Getup::FRONT;
    else
      cache_.odometry->getting_up_side_ = Getup::BACK;
  }

  // based on joint commands for head, update joint values
  if (cache_.joint_command->send_head_pitch_angle_ && cache_.joint_command->head_pitch_angle_time_ > 0){
    if (cache_.joint_command->head_pitch_angle_change_)
      cache_.joint_command->angles_[HeadPitch] += cache_.joint->values_[HeadPitch];
    float moveFrac = 1000.0* dt / cache_.joint_command->head_pitch_angle_time_;
    float moveDist = cache_.joint_command->angles_[HeadPitch] - cache_.joint->values_[HeadPitch];
    float singleMove = moveFrac*moveDist;
    cache_.joint->values_[HeadPitch] += singleMove;
  }

  // update head pan angle
  if (cache_.joint_command->send_head_yaw_angle_ && cache_.joint_command->head_yaw_angle_time_ > 0){
    if(cache_.joint_command->head_yaw_angle_change_)
      cache_.joint_command->angles_[HeadYaw] += cache_.joint->values_[HeadYaw];
    float moveFrac = 1000.0* dt / cache_.joint_command->head_yaw_angle_time_;
    cache_.joint_command->head_yaw_angle_time_ -= 1000.0* dt;
    float moveDist = cache_.joint_command->angles_[HeadYaw] - cache_.joint->values_[HeadYaw];
    float singleMove = moveFrac*moveDist;
    singleMove = min(12.0f * DEG_T_RAD, max(-12.0f * DEG_T_RAD, singleMove));
    cache_.joint->values_[HeadYaw] += singleMove;
    if (cache_.joint->values_[HeadYaw] > 120.0*DEG_T_RAD)
      cache_.joint->values_[HeadYaw] = 120.0*DEG_T_RAD;
    else if (cache_.joint->values_[HeadYaw] < -120.0*DEG_T_RAD)
      cache_.joint->values_[HeadYaw] = -120.0*DEG_T_RAD;
  }

  return false;
}


std::vector<std::string> SimulatedPlayer::getTextDebug(){
  return core->textlog_->textEntries();
}

void SimulatedPlayer::setStrategy(){
  core->interpreter_->doStrategyCalculations();
}

float SimulatedPlayer::crop(float val, float min, float max){
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

void SimulatedPlayer::setPenalty(WorldObjectBlock* simulationMem){
  if (cache_.game_state->state() == PLAYING || cache_.game_state->state() == READY){
    setPenaltyPosition(simulationMem);
    // robot gets stood up on penalty
    resetCounters();
    penaltySeconds = 30.0;
    cache_.game_state->setState(PENALISED);
    if (PRINT) cout << index_ << " is penalised" << endl;
  }
}

void SimulatedPlayer::resetCounters(){
  penaltySeconds = -0.1;
  kickSeconds = -0.1;
  kickHitSeconds = -0.1;
  getupSeconds = -0.1;
  cache_.odometry->getting_up_side_ = Getup::NONE;
  diving = Dive::NONE;
  walkToTarget = false;
  cache_.sensor->values_[angleX] = 0;
  cache_.sensor->values_[angleY] = 0;
}

void SimulatedPlayer::setPenaltyPosition(WorldObjectBlock* simulationMem){
  // affect true location in simulation mem

  WorldObject* robot = &(simulationMem->objects_[index_]);
  WorldObject* ball = &(simulationMem->objects_[WO_BALL]);
  if (ball->loc.y > 0){
    robot->loc.y = -FIELD_Y/2.0 - 100.0;
    robot->orientation = M_PI/2.0;
  } else {
    robot->loc.y = FIELD_Y/2.0 + 100.0;
    robot->orientation = -M_PI/2.0;
  }
  // check that no one is already at x
  float penX = -PENALTY_CROSS_X;
  if (team_ == TEAM_RED) penX = 1200;
  for (int i = 0; i < 4; i++){
    float desiredX = penX + i * 380.0;
    bool posTaken = false;
    for (int j = WO_TEAM1; j <= WO_OPPONENT4; j++){
      if (j == index_) continue;
      if (simulationMem->objects_[j].loc.getDistanceTo(Point2D(desiredX,robot->loc.y)) < 100){
        posTaken = true;
        break;
      }
    }
    if (!posTaken){
      robot->loc.x = desiredX;
      break;
    }
    desiredX = penX + -i * 380.0;
    posTaken = false;
    for (int j = WO_TEAM1; j <= WO_OPPONENT4; j++){
      if (j == index_) continue;
      if (simulationMem->objects_[j].loc.getDistanceTo(Point2D(desiredX,robot->loc.y)) < 100){
        posTaken = true;
        break;
      }
    }
    if (!posTaken){
      robot->loc.x = desiredX;
      break;
    }
  }
}

void SimulatedPlayer::setFallen(){
  if (PRINT) cout << index_ << " has fallen " << endl;
  // random fall direction
  float randPct = ((float)rand())/((float)RAND_MAX);
  // give a little extra tilt and roll
  if (Random::inst().sampleB()){
    cache_.sensor->values_[angleY] = 0.05;
    cache_.sensor->values_[angleX] = 0.05;
  } else {
    cache_.sensor->values_[angleY] = -0.05;
    cache_.sensor->values_[angleX] = -0.05;
  }
  int choice = Random::inst().sampleU(1,10);
  if (choice <= 4) // tilt fwd
    cache_.sensor->values_[angleY] = M_PI/2.0;
  else if (choice <= 8) // tilt back
    cache_.sensor->values_[angleY] = -M_PI/2.0;
  else if (choice <= 9) // tilt left
    cache_.sensor->values_[angleX] = -M_PI/2.0;
  else // tilt right
    cache_.sensor->values_[angleX] = M_PI/2.0;
}

void SimulatedPlayer::setRole(Role role) {
  cache_.robot_state->role_ = role;
}
