#include "BehaviorSimulation.h"

// core
#include <VisionCore.h>
#include <communications/CommunicationModule.h>

#include <common/RobotPositions.h>

// memory
#include <memory/GameStateBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/MemoryFrame.h>
#include <memory/BehaviorBlock.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/SensorBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/BehaviorParamBlock.h>

#include <stdlib.h>
#include <InterpreterModule.h>

#define SKIP_TO_PLAYING false
#define OUR_TEAM 1
#define THEIR_TEAM -1

using std::vector;
using std::string;


BehaviorSimulation::BehaviorSimulation(bool lmode) : lmode_(lmode) {
  Random::engine() == std::mt19937(0);
  for (int i = WO_PLAYERS_FIRST; i <= WO_ROBOTS_LAST; i++){
    sims_[i] = nullptr;
    fallenTime[i] = 0;
  }

  currentSim = 0;
  lastKick = 0;
  lastKickX = 0;
  ballClearedFromCircle = false;

  simInfo = "";

  simBlueScore = 0;
  simRedScore = 0;
  simPenaltyKick =false;
  numHalves = 0;
  PRINT = false;
  forceManualPositions = false;
  forceDesiredPositions = false;
  simTimer = 45;
  // start at 655 to include the 5 init, 45 ready and 5 set seconds before kickoff
  halfTimer = 655;

  simPenaltyKick = false;
  timeInc = 1.0/30.0;

  // init our world object mem
  memory_ = std::make_unique<MemoryFrame>(false, MemoryOwner::VISION, 0, 1);
  gtcache.fill(memory_.get(), {"world_objects", "game_state", "vision_frame_info"});
  gtcache.world_object->init(TEAM_BLUE);
  gtcache.world_object->reset();
  gtcache.game_state->isPenaltyKick = simPenaltyKick;
  gtcache.frame_info->frame_id = 0;
  gtcache.frame_info->seconds_since_start = 0;

  loadGame("behavior");
  auto& gtconfig = config_.objects.gtconfig;

  std::vector<MemoryCache> playerCaches;
  for(int i = WO_PLAYERS_FIRST; i <= WO_ROBOTS_LAST; i++) {
    if(gtconfig.contains(i)) {
      if(i <= WO_TEAM5) {
        currentSim = i;
      }
      int teamsign, self;
      getTeamSignAndSelf(i, teamsign, self);
      int team = teamsign >= 0 ? TEAM_BLUE : TEAM_RED;
      sims_[i] = std::make_unique<SimulatedPlayer>(team, self, lmode_);
      auto itrole = config_.roles.find(i);
      if(itrole != config_.roles.end())
        sims_[i]->setRole(itrole->second);
      playerCaches.push_back(sims_[i]->getMemoryCache());
    }
  }

  switch(config_.game_state) {
    case PLAYING:
      simTimer = 5 * 60.0f; break;
    default:
      simTimer = 5.0f; break;
  }

  gtcache.world_object->objects_[WO_BALL].loc = Point2D(0,0);

  for (int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
    int teamsign;
    int self;
    getTeamSignAndSelf(i,teamsign,self);
   
    if(SKIP_TO_PLAYING) {
      if(teamsign == OUR_TEAM)
        setObjectFromPose(i,teamsign,&RobotPositions::ourKickoffPosesDesired[self]);
      else
        setObjectFromPose(i,teamsign,&RobotPositions::theirKickoffPosesDesired[self]);
    }
    else setObjectFromPose(i,teamsign,&RobotPositions::startingSidelinePoses[self]);

    // special locations for penalty kick
    if (simPenaltyKick) {
      if (i == WO_TEAM_LAST){
        gtcache.world_object->objects_[i].loc.x = PENALTY_CROSS_X - 1000;
        gtcache.world_object->objects_[i].loc.y = 0;
        gtcache.world_object->objects_[i].orientation = 0;
        gtcache.world_object->objects_[WO_BALL].loc = Point2D(PENALTY_CROSS_X,0);
      } else {
        gtcache.world_object->objects_[i].loc.x = FIELD_X/2.0 - 50;
        gtcache.world_object->objects_[i].loc.y = 0;
        gtcache.world_object->objects_[i].orientation = M_PI;
      }
    }

    // set way off field if not active
    if (sims_[i] == nullptr) {
      gtcache.world_object->objects_[i].loc.x = 10000;
      gtcache.world_object->objects_[i].loc.y = 10000;
    }

  }
  applyConfig(gtcache, playerCaches);
  for (int i = WO_PLAYERS_FIRST; i <= WO_ROBOTS_LAST; i++){
    if(sims_[i] != nullptr)
      sims_[i]->initLocalization();
  }
}



BehaviorSimulation::~BehaviorSimulation(){
  // core will delete memory for us
  gtcache.world_object = NULL;
  gtcache.game_state = NULL;
}

void BehaviorSimulation::doPenaltyKickReset(){
  gtcache.world_object->objects_[WO_BALL].loc = Point2D(PENALTY_CROSS_X,0);
  gtcache.world_object->objects_[WO_BALL].absVel = Point2D(0,0);
  gtcache.world_object->objects_[WO_TEAM_LAST].loc = Point2D(PENALTY_CROSS_X-1000,0);
  gtcache.world_object->objects_[WO_TEAM_LAST].orientation = 0;
  gtcache.world_object->objects_[WO_OPPONENT_FIRST].loc = Point2D(FIELD_X/2.0-50,0);
  gtcache.world_object->objects_[WO_OPPONENT_FIRST].orientation = M_PI;
  changeSimulationState(READY);
  simTimer = 5.0;

  if (sims_[WO_TEAM_LAST] != nullptr){
    sims_[WO_TEAM_LAST]->resetCounters();
    sims_[WO_TEAM_LAST]->cache_.game_state->setState(READY);
  }
  if (sims_[WO_OPPONENT_FIRST] != nullptr){
    sims_[WO_OPPONENT_FIRST]->resetCounters();
    sims_[WO_OPPONENT_FIRST]->cache_.game_state->setState(READY);
  }
}

void BehaviorSimulation::stepTimer(Point2D &ballLoc,Point2D &ballVel) {
  simTimer -= timeInc;
  halfTimer -= timeInc;
  gtcache.game_state->secsRemaining = (int)halfTimer;
  gtcache.frame_info->seconds_since_start += timeInc;
  gtcache.frame_info->frame_id++;

#ifdef TOOL
  if (simTimer < 10 && simPenaltyKick && sims_[WO_TEAM_LAST]->cache_.game_state->state() == PLAYING){
    simInfo += std::to_string(simTimer) + " seconds left in PK, ";
  }
#endif

  if (simTimer <= 0){
    // different for penalty mode
    if (simPenaltyKick){
      // go through ready,set,pen
      if (gtcache.game_state->state() == INITIAL){
        changeSimulationState(READY);
        simTimer = 5.0;
      } else if (gtcache.game_state->state() == READY){
        ballLoc = Point2D(PENALTY_CROSS_X,0);
        // go to set
        changeSimulationState(SET);
        simTimer = 5.0;
      } else if (gtcache.game_state->state() == SET){
        ballLoc = Point2D(PENALTY_CROSS_X,0);
        // go to playing
        changeSimulationState(PLAYING);
        simTimer = 60.0;
        halfTimer = 60.0;
      } else if (gtcache.game_state->state() == PENALISED){
        ballLoc = Point2D(PENALTY_CROSS_X,0);
        // go to playing
        changeSimulationState(PLAYING);
        simTimer = 60.0;
        halfTimer = 60.0;
      } else if (gtcache.game_state->state() == PLAYING){
        // TIME IS UP!
#ifdef TOOL
        simInfo += "PK TIME UP, NO GOAL SCORED!, ";
#endif
        doPenaltyKickReset();
      }
      ballClearedFromCircle = true;
    } else {
      if (gtcache.game_state->state() == INITIAL){
        // go to ready
        ballLoc = Point2D(0,0);
        ballVel = Point2D(0,0);
        changeSimulationState(READY);
        simTimer = 45.0;
      } else if (gtcache.game_state->state() == READY){
        ballLoc = Point2D(0,0);
        ballVel = Point2D(0,0);

        // go to set
        changeSimulationState(SET);
        simTimer = 5.0;
      } else if (gtcache.game_state->state() == SET){
        changeSimulationState(PLAYING);
        if (halfTimer > 600) halfTimer = 600;
        simTimer = halfTimer;
      }
      // back to ready
      else if (gtcache.game_state->state() == PLAYING){
        cout << "Half: " << numHalves << " Blue: " << simBlueScore << ", Red: " << simRedScore << endl;
        numHalves++;
        changeSimulationKickoff();

        ballLoc = Point2D(0,0);
        ballVel = Point2D(0,0);
        changeSimulationState(READY);
        simTimer = 45.0;
        halfTimer = 650.0;
      }
    }
  }
}
  
void BehaviorSimulation::stepCheckGoal(Point2D &ballLoc, Point2D &ballVel) {
  // check for goal
  if (fabs(ballLoc.x) > FIELD_X/2.0 && fabs(ballLoc.y) < GOAL_Y/2.0){
    if (ballLoc.x > 0){
      // if from circle and our kickoff, not valid
      if (gtcache.game_state->ourKickOff && !ballClearedFromCircle){
#ifdef TOOL
        simInfo += "KICKOFF SHOT BLUE, ";
#endif
      } else {
#ifdef TOOL
        simInfo += "GOAL BLUE, ";
#endif
        setSimScore(true);
        //cout << sims_[3]->gtcache.frame_info->frame_id << " Goal blue. Score Blue: " << simBlueScore << " Red: " << simRedScore << endl;
      }
      gtcache.game_state->ourKickOff = false;
    }
    else {
      // if from circle and our kickoff, not valid
      if (!gtcache.game_state->ourKickOff && !ballClearedFromCircle){
#ifdef TOOL
        simInfo += "KICKOFF SHOT RED, ";
#endif
      } else {
#ifdef TOOL
        simInfo += "GOAL RED, ";
#endif
        setSimScore(false);
        //cout << sims_[3]->gtcache.frame_info->frame_id << " Goal red. Score Blue: " << simBlueScore << " Red: " << simRedScore << endl;
      }
      gtcache.game_state->ourKickOff = true;
    }
    ballLoc = Point2D(0,0);
    ballVel = Point2D(0,0);
    // go to ready
    simTimer = 45.0;
    changeSimulationState(READY);
    if (simPenaltyKick){
      doPenaltyKickReset();
      ballLoc = gtcache.world_object->objects_[WO_BALL].loc;
      ballVel = gtcache.world_object->objects_[WO_BALL].absVel;
    }
  }
}

void BehaviorSimulation::stepCheckBounds(Point2D &ballLoc, Point2D &ballVel) {
  // check for out of bounds
  if(fabs(ballLoc.x) >= 9000 && fabs(ballLoc.y) >= 9000) {
    // The ball is being intentionally hidden
    return;
  }
  if (fabs(ballLoc.y) > FIELD_Y/2.0){
    if (PRINT) cout << "ball out at " << ballLoc << endl;
    ballVel = Point2D(0,0);
    if (ballLoc.y > 0)
      ballLoc.y = FIELD_Y/2.0 - 400;
    else
      ballLoc.y = -FIELD_Y/2.0 + 400;
    if (lastKick == TEAM_BLUE){
#ifdef TOOL
      simInfo += "OUT OF BOUNDS on BLUE, ";
#endif
      if (ballLoc.x < lastKickX) {
        ballLoc.x -= 1000;
      } else {
        ballLoc.x = lastKickX - 1000;
      }
    } else {
#ifdef TOOL
      simInfo += "OUT OF BOUNDS on RED, ";
#endif
      // if (ballLoc.x > lastKickX) {
      //   ballLoc.x += 1000;
      // } else {
      //   ballLoc.x = lastKickX + 1000;
      // }
      ballLoc.x = -PENALTY_X;
      if (ballLoc.y > 0)
        ballLoc.y = GOAL_Y / 2;
      else
        ballLoc.y = -GOAL_Y / 2;
      gtcache.game_state->isFreeKick = true;
      gtcache.game_state->isFreeKickTypeGoal = true;
      gtcache.game_state->ourKickOff = true;    
    }
    // max and min values
    if (ballLoc.x < -2000) ballLoc.x = -2000;
    if (ballLoc.x > 2000) ballLoc.x = 2000;
  }

  if (fabs(ballLoc.x) > FIELD_X/2.0){
    if (PRINT) cout << "ball out at " << ballLoc << endl;
    ballVel = Point2D(0,0);
    if (ballLoc.y > 0)
      ballLoc.y = FIELD_Y/2.0 - 400;
    else
      ballLoc.y = -FIELD_Y/2.0 + 400;
    if (lastKick == TEAM_BLUE){
#ifdef TOOL
      simInfo += "OUT ENDLINE on BLUE, ";
#endif
      // if (ballLoc.x > 0){
      //   ballLoc.x = 0;
      //   if (PRINT) cout << "Ball out endline on blue" << endl;
      // } else {
      //   ballLoc.x = -2000;
      // }
      ballLoc.x = FIELD_X / 2.0 - PENALTY_X;
      if (ballLoc.y > 0)
        ballLoc.y = GOAL_Y / 2;
      else
        ballLoc.y = -GOAL_Y / 2;
      gtcache.game_state->isFreeKick = true;
      gtcache.game_state->isFreeKickTypeGoal = true;
      gtcache.game_state->ourKickOff = false;
    } else {
#ifdef TOOL
      simInfo += "OUT ENDLINE on RED, ";
#endif
      ballLoc.x = -FIELD_X / 2.0 + PENALTY_X;
      if (ballLoc.y > 0)
        ballLoc.y = GOAL_Y / 2;
      else
        ballLoc.y = -GOAL_Y / 2;
      gtcache.game_state->isFreeKick = true;
      gtcache.game_state->isFreeKickTypeGoal = true;
      gtcache.game_state->ourKickOff = false;
      // if (ballLoc.x > 0){
      //   ballLoc.x = 2000;
      // } else {
      //   ballLoc.x = 0;
      //   if (PRINT) cout << "Ball out endline on red" << endl;
      // }
    }
  }
}


void BehaviorSimulation::stepCheckBallCollisions(Point2D &ballLoc, Point2D &ballVel) {
  // see if ball hit anyone
  for (int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
    if(sims_[i] == nullptr) continue;
    float ballDist = 0;
    ballDist = ballLoc.getDistanceTo(gtcache.world_object->objects_[i].loc);

    if (ballDist < 120){
      // slow down ball
      if (i < WO_OPPONENT_FIRST){
#ifdef TOOL
        simInfo += "BALL HIT BLUE, ";
#endif
        lastKick = TEAM_BLUE;
        lastKickX = gtcache.world_object->objects_[i].loc.x;
      } else {
#ifdef TOOL
        simInfo += "BALL HIT RED, ";
#endif
        lastKick = TEAM_RED;
        lastKickX = -gtcache.world_object->objects_[i].loc.x;
      }
      //cout << "ball hit " << lastKick << " at " << lastKickX << endl;
    }

    // special check for keeper
    Point2D teamBallLoc = ballLoc;
    if (fabs(sims_[i]->cache_.sensor->values_[angleY]) > (M_PI/2.0 - 0.08)){
      Point2D robotLoc = gtcache.world_object->objects_[i].loc;
      bool yMatch = false;
      if (sims_[i]->cache_.sensor->values_[angleY] > 0){
        yMatch = (teamBallLoc.y < robotLoc.y && teamBallLoc.y > robotLoc.y-480);
      } else {
        yMatch = (teamBallLoc.y > robotLoc.y && teamBallLoc.y < robotLoc.y+480);
      }
      if (yMatch && fabs(teamBallLoc.x - robotLoc.x) < 50){
        if (i < WO_OPPONENT_FIRST){
#ifdef TOOL
          simInfo += "BLUE KEEPER SAVE!, ";
#endif
          lastKick = TEAM_BLUE;
          lastKickX = gtcache.world_object->objects_[i].loc.x;
        } else {
#ifdef TOOL
          simInfo += "RED KEEPER SAVE!, ";
#endif
          lastKick = TEAM_RED;
          lastKickX = gtcache.world_object->objects_[i].loc.x;
        }
        //cout << "ball saved by " << lastKick << " at " << lastKickX << endl;
        if (Random::inst().sampleB() && !simPenaltyKick){
          // keeper sweep
          if (ballLoc.x < 0){
            ballVel.x = 1000;
          } else {
            ballVel.x = -1000;
          }
        } else {
          // bounced off
          ballVel.x = -ballVel.x;
          ballVel *= 0.5;
        }
      }
    }
  }

}

void BehaviorSimulation::stepPlayerFallen(int i) {
  // check how long each robot has been down for
  if (fabs(sims_[i]->cache_.sensor->values_[angleX]) > 0.8 || fabs(sims_[i]->cache_.sensor->values_[angleY]) > 0.8)
    fallenTime[i] += timeInc;
  else
    fallenTime[i] = 0;

  // penalty if they have not started getting up after 5 seconds
  if (fallenTime[i] > 5.0 && sims_[i]->cache_.odometry->getting_up_side_ == Getup::NONE){
#ifdef TOOL
    if (i < WO_OPPONENT_FIRST)
      simInfo += "INACTIVE ROBOT BLUE "+std::to_string(sims_[i]->self_)+", ";
    else
      simInfo += "INACTIVE ROBOT RED "+std::to_string(sims_[i]->self_)+", ";
#endif

    sims_[i]->setPenalty(gtcache.world_object);
  }
}

void BehaviorSimulation::stepPlayerBounds(int i) {
  // penalty if left field
  if (fabs(gtcache.world_object->objects_[i].loc.x) > (GRASS_X/2.0 + 500.0) || fabs(gtcache.world_object->objects_[i].loc.y) > (GRASS_Y/2.0 + 500.0)){
#ifdef TOOL
    if (i < WO_OPPONENT_FIRST)
      simInfo += "LEAVING FIELD BLUE "+std::to_string(sims_[i]->self_)+", ";
    else
      simInfo += "LEAVING FIELD RED "+std::to_string(sims_[i]->self_)+", ";
#endif
    sims_[i]->setPenalty(gtcache.world_object);
  }
}

void BehaviorSimulation::stepPlayerKick(int i) {
  lastKick = sims_[i]->team_;
  lastKickX = gtcache.world_object->objects_[i].loc.x;
  if (gtcache.world_object->objects_[WO_BALL].loc.getMagnitude() > CIRCLE_RADIUS){
    ballClearedFromCircle = true;
  }
#ifdef TOOL
  if (lastKick == TEAM_BLUE){
    simInfo += "BLUE KICK, ";
  } else {
    simInfo += "RED KICK, ";
  }
#endif
  //cout << "ball kicked by " << lastKick << " at " << lastKickX << endl;

  // if its a penalty kick, this could be illegal
  if (simPenaltyKick) {
    if (lastKick == TEAM_BLUE){
      Point2D ballLoc = gtcache.world_object->objects_[WO_BALL].loc;
      if (ballLoc.x > (FIELD_X/2.0 - PENALTY_X) && fabs(ballLoc.y < PENALTY_Y/2.0)){
#ifdef TOOL
        simInfo += "BLUE TOUCHED BALL INSIDE PEN BOX, NO GOAL, ";
#endif
        doPenaltyKickReset();
      }
    } else {
      Point2D ballLoc = sims_[i]->cache_.world_object->objects_[WO_BALL].loc;
      if (ballLoc.x > (-FIELD_X/2.0 + PENALTY_X) || fabs(ballLoc.y > PENALTY_Y/2.0)){
#ifdef TOOL
        simInfo += "RED TOUCHED BALL OUTSIDE PEN BOX, GOAL, ";
#endif
        setSimScore(true);
        doPenaltyKickReset();
      }
    }
  } // pen kick
}
  
void BehaviorSimulation::stepPlayerBumpBall(int /*i*/, Point2D &ballLoc, Point2D &ballVel, WorldObject *robot) {
  // check if we bumped ball (dribble)
  if (ballLoc.getDistanceTo(robot->loc) < 60){
    // add up to 30 degree error
    float headingError = Random::inst().sampleU(-.5,.5) * 60 * DEG_T_RAD;
    float velError = Random::inst().sampleU(-.5,.5) * 500;

    Point2D absVel(250+velError, robot->orientation+headingError, POLAR);
    Point2D disp(100, robot->orientation, POLAR);

    gtcache.world_object->objects_[WO_BALL].loc += disp;
    gtcache.world_object->objects_[WO_BALL].absVel = absVel;
    ballVel = absVel;
    ballLoc = gtcache.world_object->objects_[WO_BALL].loc;
  }
}

void BehaviorSimulation::stepPlayerPenaltyBox(int i, WorldObject *robot) {
  // check if we went into own penalty box
  if (sims_[i]->self_ != 1 && i <= WO_TEAM_LAST && robot->loc.x < (-FIELD_X/2.0 + PENALTY_X) && fabs(robot->loc.y) < PENALTY_Y/2.0){
    sims_[i]->setPenalty(gtcache.world_object);
#ifdef TOOL
    simInfo += "ILLEGAL DEFENDER BLUE " + std::to_string(sims_[i]->self_) + ", ";
#endif
  }
  if (sims_[i]->self_ != 1 && i >= WO_OPPONENT_FIRST && robot->loc.x > (FIELD_X/2.0 - PENALTY_X) && fabs(robot->loc.y) < PENALTY_Y/2.0){
    sims_[i]->setPenalty(gtcache.world_object);
#ifdef TOOL
    simInfo += "ILLEGAL DEFENDER RED " + std::to_string(sims_[i]->self_) + ", ";
#endif
  }

}

void BehaviorSimulation::stepPlayerCollisions(int i, WorldObject *robot) {
  if (SimulatedPlayer::DEBUGGING_POSITIONING)
    return;
  // check if we hit other robots
  for (int j = 1; j <= WO_OPPONENT_LAST; j++){
    if (i == j) continue;
    if(sims_[i] == nullptr) continue;
    // check if we ran into another robot
    float mateDist = robot->loc.getDistanceTo(gtcache.world_object->objects_[j].loc);
    if (mateDist < 180 && sims_[j]->cache_.game_state->state() != PENALISED && sims_[i]->cache_.game_state->state() != PENALISED){
      sims_[i]->setPenalty(gtcache.world_object);
#ifdef TOOL
      if (sims_[i]->team_ == TEAM_BLUE){
        simInfo += "PUSHING BLUE " + std::to_string(sims_[i]->self_) + ", ";
      } else {
        simInfo += "PUSHING RED " + std::to_string(sims_[i]->self_) + ", ";
      }
#endif
    }
  }
}

void BehaviorSimulation::stepPlayerComm(int i) {
  if (sims_[i]->cache_.frame_info->frame_id % 6 != 0)
    return;
  // get team packet we would have sent
  // and update other teammates
  auto sent = &(sims_[i]->cache_.team_packets->relayData[sims_[i]->self_]);
  // send team pkts to other robots
  for (int j = 1; j <= WO_OPPONENT_LAST; j++){
    // basically we "send" our team packets across this way
    if(sims_[j] != nullptr && (sims_[i]->team_ == sims_[j]->team_)) {
      int robotNumber = sims_[i]->self_;
      int idx = sims_[j]->team_ == TEAM_RED ? j - WO_TEAM_LAST : j;
      sims_[j]->cache_.team_packets->relayData[robotNumber] = *sent;
      sims_[j]->cache_.team_packets->frameReceived[robotNumber] = sims_[j]->cache_.frame_info->frame_id;
      sims_[j]->cache_.team_packets->ballUpdated[robotNumber] = sims_[j]->cache_.frame_info->frame_id;
      sims_[j]->cache_.team_packets->oppUpdated[robotNumber] = true;

      if (lmode_ && idx != robotNumber) {
        // Populate world objects for team mate position
        sims_[j]->cache_.world_object->objects_[robotNumber].loc.x = sent->locData.robotX;
        sims_[j]->cache_.world_object->objects_[robotNumber].loc.y = sent->locData.robotY;
        sims_[j]->cache_.world_object->objects_[robotNumber].orientation = sent->locData.orientation();

        sims_[j]->cache_.world_object->objects_[robotNumber].sd.x = sent->locData.robotSDX;
        sims_[j]->cache_.world_object->objects_[robotNumber].sd.y = sent->locData.robotSDY;
        sims_[j]->cache_.world_object->objects_[robotNumber].sdOrientation = sent->locData.sdOrient;
      }
    }
  } // for
}


void BehaviorSimulation::simulationStep(){
  std::string oldSimInfo = simInfo;
  simInfo = "";

  physics_.setObjects(gtcache.world_object);
  physics_.step();
  Point2D ballVel = gtcache.world_object->objects_[WO_BALL].absVel;
  Point2D ballLoc = gtcache.world_object->objects_[WO_BALL].loc;
  
  stepTimer(ballLoc,ballVel);

  stepCheckGoal(ballLoc,ballVel);
  stepCheckBounds(ballLoc,ballVel);
  //stepCheckBallCollisions(ballLoc,ballVel); // let the physics simulator do this

  // save new loc and vel back to wo
  gtcache.world_object->objects_[WO_BALL].absVel = ballVel;
  gtcache.world_object->objects_[WO_BALL].loc = ballLoc;


  // step the simulation ahead
  for (int i = WO_PLAYERS_FIRST; i <= WO_ROBOTS_LAST; i++){
    if(sims_[i] == nullptr)
      continue;
    stepPlayerFallen(i);
    stepPlayerBounds(i);
    bool kicked = sims_[i]->processFrame(gtcache.world_object,gtcache.game_state);
    if (kicked)
      stepPlayerKick(i);

    // get updates of where we are and where ball is, and update other bots with it
    // get new ball location
    ballLoc = gtcache.world_object->objects_[WO_BALL].loc;
    ballVel = gtcache.world_object->objects_[WO_BALL].absVel;
    // get new robot location
    WorldObject* robot = &(gtcache.world_object->objects_[i]);
    if(i == WO_TEAM_COACH) {
      auto& srobot = sims_[i]->cache_.world_object->objects_[i];
      robot->loc = srobot.loc;
      robot->height = srobot.height;
      robot->orientation = srobot.orientation;
    } else {
      //stepPlayerBumpBall(i,ballLoc,ballVel,robot); // let the physics simulator do this
      stepPlayerPenaltyBox(i,robot);
      stepPlayerCollisions(i,robot);
      stepPlayerComm(i);
    }
  } // simulate step for this robot

  // keep old text around until we have new text
#ifdef TOOL
  if (simInfo == "") simInfo = oldSimInfo;
#endif
}

void BehaviorSimulation::getTeamSignAndSelf(int i, int &teamsign, int &self) {
  self = i;
  teamsign = OUR_TEAM;
  if (self > WO_TEAM_LAST && self != WO_TEAM_COACH) {
    self -= WO_TEAM_LAST;
    teamsign = THEIR_TEAM;
  }
}

void BehaviorSimulation::setObjectFromPose(int i, int teamsign, const Pose2D *pose) {
  gtcache.world_object->objects_[i].loc.x = teamsign * pose->translation.x;
  gtcache.world_object->objects_[i].loc.y = teamsign * pose->translation.y;
  if (teamsign > 0) {
    gtcache.world_object->objects_[i].orientation = pose->rotation;
  } else {
    gtcache.world_object->objects_[i].orientation = normalizeAngle(M_PI + pose->rotation);
  }
}

std::vector<std::string> BehaviorSimulation::getTextDebug(int index){
  if(index < 1 || index >= sims_.size())
    return vector<string>();
  return sims_[index]->getTextDebug();
}

void BehaviorSimulation::changeSimulationState(State state){
  gtcache.game_state->setState(state);
  ballClearedFromCircle = false;

  if (state == SET && !simPenaltyKick){
    gtcache.world_object->objects_[WO_BALL].loc = Point2D(0,0);
    gtcache.world_object->objects_[WO_BALL].absVel = Point2D(0,0);
    // check positions
    //int blueCircleCount = 0;
    //int redCircleCount = 0;
    for (int i = WO_OPPONENT_LAST; i > 0; i--){
      if(sims_[i] == nullptr) continue;
      bool manual = false;
      float xLineKick = -50;

      if (i < WO_OPPONENT_FIRST){
        float xLine = xLineKick;

        // past x barrier
        if (gtcache.world_object->objects_[i].loc.x > xLine){
          manual = true;
        }
        // robot inside circle if not our kickoff
        if (gtcache.world_object->objects_[i].loc.getMagnitude() < CIRCLE_RADIUS){
          if (!gtcache.game_state->ourKickOff)
            manual = true;
        }
        // keeper not in box
        if (i == 1 && (gtcache.world_object->objects_[i].loc.x > (-FIELD_X/2.0 + PENALTY_X) || fabs(gtcache.world_object->objects_[i].loc.y) > (PENALTY_Y/2.0))){
          manual = true;
        }
        // other player in box
        if (i != 1 && (gtcache.world_object->objects_[i].loc.x < (-FIELD_X/2.0 + PENALTY_X) && fabs(gtcache.world_object->objects_[i].loc.y) < (PENALTY_Y/2.0))){
          manual = true;
        }
        // off field
        if (fabs(gtcache.world_object->objects_[i].loc.x) > (FIELD_X/2.0) || fabs(gtcache.world_object->objects_[i].loc.y) > (FIELD_Y/2.0)){
          manual = true;
        }
      } else { // team red
        float xLine = -xLineKick;

        // past x barrier
        if (gtcache.world_object->objects_[i].loc.x < xLine){
          manual = true;
        }
        // robot inside circle and not our kickoff
        if (gtcache.world_object->objects_[i].loc.getMagnitude() < CIRCLE_RADIUS){
          if (gtcache.game_state->ourKickOff)
            manual = true;
        }
        // keeper not in box
        if (i == WO_OPPONENT_FIRST && (gtcache.world_object->objects_[i].loc.x < (FIELD_X/2.0 - PENALTY_X) || fabs(gtcache.world_object->objects_[i].loc.y) > (PENALTY_Y/2.0))){
          manual = true;
        }
        // other player in box
        if (i != WO_OPPONENT_FIRST && (gtcache.world_object->objects_[i].loc.x > (FIELD_X/2.0 - PENALTY_X) && fabs(gtcache.world_object->objects_[i].loc.y) < (PENALTY_Y/2.0))){
          manual = true;
        }
        // off field
        if (fabs(gtcache.world_object->objects_[i].loc.x) > (FIELD_X/2.0) || fabs(gtcache.world_object->objects_[i].loc.y) > (FIELD_Y/2.0)){
          manual = true;
        }
      } // team

      if (manual || forceManualPositions || forceDesiredPositions) {
#ifdef TOOL
        if (i < WO_OPPONENT_FIRST)
          simInfo += "ILLEGAL POSITION BLUE" + std::to_string(i) + ", ";
        else
          simInfo += "ILLEGAL POSITION RED" + std::to_string(i-WO_TEAM_LAST) + ", ";
#endif
        int self;
        int teamsign;
        getTeamSignAndSelf(i,teamsign,self);
        
        const Pose2D *pose;
        if (((i <= WO_TEAM_LAST) &&  gtcache.game_state->ourKickOff) ||
            ((i >  WO_TEAM_LAST) && !gtcache.game_state->ourKickOff)) {
          if (forceDesiredPositions)
            pose = RobotPositions::ourKickoffPosesDesired + self;
          else
            pose = RobotPositions::ourKickoffPosesManual + self;
        } else {
          if (forceDesiredPositions)
            pose = RobotPositions::theirKickoffPosesDesired + self;
          else
            pose = RobotPositions::theirKickoffPosesManual + self;
        }
        setObjectFromPose(i,teamsign,pose);
/*
        if (i == WO_TEAM_FIRST || i == WO_OPPONENT_FIRST)
          gtcache.world_object->objects_[i].loc = Point2D(-FIELD_X/2.0, 0);
        if ((i < 5 && gtcache.game_state->ourKickOff) || (i > 4 && !gtcache.game_state->ourKickOff)){
          if (i == 2 || i == 6)
            gtcache.world_object->objects_[i].loc = Point2D(-PENALTY_CROSS_X, GOAL_Y/2.0);
          if (i == 3 || i == 7)
            gtcache.world_object->objects_[i].loc = Point2D(-FIELD_X/2.0+PENALTY_X+200, -PENALTY_Y/2.0);
          if (i == 4 || i == 8)
            gtcache.world_object->objects_[i].loc = Point2D(-CIRCLE_RADIUS-100, 0);
        } else {
          // their kickoff
          float xPos = -FIELD_X/2.0+PENALTY_X+200;
          if (i == 2 || i == 6)
            gtcache.world_object->objects_[i].loc = Point2D(xPos, (FIELD_Y+PENALTY_Y)/4.0);
          if (i == 3 || i == 7)
            gtcache.world_object->objects_[i].loc = Point2D(xPos, (FIELD_Y+PENALTY_Y)/-4.0);
          if (i == 4 || i == 8)
            gtcache.world_object->objects_[i].loc = Point2D(xPos, GOAL_Y/2.0);
        }

        if (i > 4){
          gtcache.world_object->objects_[i].loc = -gtcache.world_object->objects_[i].loc;
          gtcache.world_object->objects_[i].orientation = M_PI;
        }
*/
      } // manual
    } // loop
  }

  simTimer = 45.0;
  if (gtcache.game_state->state() == SET){
    simTimer = 5.0;
  } else if (gtcache.game_state->state() == PLAYING){
    if (simPenaltyKick){
      simTimer = 60.0;
      halfTimer = 60.0;
    } else {
      simTimer = halfTimer;
    }
  }

  if (gtcache.game_state->state() == INITIAL){
    for (int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
      if(sims_[i] != nullptr)
        sims_[i]->resetCounters();
    }
  }
  forceManualPositions = false;
  forceDesiredPositions = false;
}


void BehaviorSimulation::moveRobot(int index, AngRad rotation, Point2D translation){
  if (index < 1 || index > WO_OPPONENT_LAST) return;
  WorldObject* robot = &(gtcache.world_object->objects_[index]);
  robot->loc += translation;
  robot->orientation += rotation;
}

void BehaviorSimulation::moveBall(Point2D pos){
  WorldObject* ball = &(gtcache.world_object->objects_[WO_BALL]);
  ball->loc = pos;
}

void BehaviorSimulation::teleportBall(Point2D pos){
  WorldObject* ball = &(gtcache.world_object->objects_[WO_BALL]);
  ball->loc = pos;
}

void BehaviorSimulation::changeSimulationKickoff(){
  gtcache.game_state->ourKickOff = !gtcache.game_state->ourKickOff;
}

void BehaviorSimulation::restartSimulationInterpreter(){
  for (int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
    if(sims_[i] == nullptr) continue;
    sims_[i]->core->interpreter_->restart();
  }
}

void BehaviorSimulation::setSimScore(bool blue){
  if (blue) simBlueScore++;
  else simRedScore++;
  gtcache.game_state->ourScore = simBlueScore;
  gtcache.game_state->opponentScore = simRedScore;
}


void BehaviorSimulation::setPenalty(int index){
  if (index < 1 || index > WO_OPPONENT_LAST) return;
  sims_[index]->setPenalty(gtcache.world_object);
}

void BehaviorSimulation::flipRobot(int index){
  if (index < 1 || index > WO_OPPONENT_LAST) return;
  gtcache.world_object->objects_[index].loc = -gtcache.world_object->objects_[index].loc;
  gtcache.world_object->objects_[index].orientation = normalizeAngle(gtcache.world_object->objects_[index].orientation + M_PI);
}

std::string BehaviorSimulation::getSimInfo(){
  return simInfo;
}

void BehaviorSimulation::setFallen(int index){
  if(index < 1 || index >= sims_.size()) return;
  if(sims_[index] == nullptr) return;

  sims_[index]->setFallen();
}

void BehaviorSimulation::kickBall(){
  WorldObject* ball = &(gtcache.world_object->objects_[WO_BALL]);
  // random target within goal
  float targetY = Random::inst().sampleU(-.5,.5) * 1400;
  float targetX = -3000;
  if (ball->loc.x > 0){
    targetX = 3000;
  }
  Point2D absVel(3500, ball->loc.getAngleTo(Point2D(targetX, targetY)), POLAR);
  ball->absVel = absVel;
}

// check if anyone's localization is off and by how much
int BehaviorSimulation::checkLocalizationErrors(){
  PRINT = true;

  for(int i = WO_PLAYERS_FIRST; i <= WO_PLAYERS_LAST; i++){
    if(sims_[i] == nullptr) continue;
    sims_[i]->PRINT = true;

    // not while the robot is getting up
    if (sims_[i]->cache_.odometry->getting_up_side_ != Getup::NONE) continue;
    // not if penalized
    if (sims_[i]->cache_.game_state->state() == PENALISED) continue;
    // not in set if keeper
    if (sims_[i]->cache_.game_state->state() == SET && sims_[i]->self_ == 1) continue;

    WorldObject* trueRobot = &(gtcache.world_object->objects_[i]);
    WorldObject* belRobot = &(sims_[i]->cache_.world_object->objects_[sims_[i]->self_]);

    float distance, orient;
    if (i < WO_OPPONENT_FIRST){
      distance = trueRobot->loc.getDistanceTo(belRobot->loc);
      orient   = fabs(normalizeAngle(trueRobot->orientation - belRobot->orientation));
      //cout << i << " true location: " << trueRobot->loc << " ori: " << RAD_T_DEG* trueRobot->orientation << " belief: " << belRobot->loc << " ori: " << RAD_T_DEG* belRobot->orientation << " error: "<< distance << ", " << orient *RAD_T_DEG << endl;

    } else {
      distance = trueRobot->loc.getDistanceTo(-belRobot->loc);
      orient   = fabs(normalizeAngle(trueRobot->orientation + M_PI - belRobot->orientation));
      //cout << i << " true location: " << trueRobot->loc << " ori: " << RAD_T_DEG* trueRobot->orientation << " belief: " << -belRobot->loc << " ori: " << RAD_T_DEG* normalizeAngle(M_PI+belRobot->orientation) << " error: "<< distance << ", " << orient *RAD_T_DEG << endl;

    }

    if (distance > 500 || orient > (DEG_T_RAD*25.0) || isnan(belRobot->loc.x) || isnan(belRobot->loc.y) || isnan(belRobot->sd.x) || isnan(belRobot->sd.y) || isnan(belRobot->orientation) || isnan(belRobot->sdOrientation)){
      cout << i << " error: "<< distance << ", " << orient *RAD_T_DEG << endl;
      return i;
    }
  }
  return 0;
}

//////////////////////////////
// behavior simulation stuff
//////////////////////////////


void BehaviorSimulation::runParamTests(){
  for (float diff = -10.0*DEG_T_RAD; diff < 12*DEG_T_RAD; diff += 5.0*DEG_T_RAD){

    cout << endl << endl << "TEST with diff " << diff*RAD_T_DEG << endl << endl;

    // set all players with this diff
    for (int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++){
      cout << "i is " << i << endl;
      if(sims_[i] == nullptr) continue;
      cout << " and setting " << i << endl;
      sims_[i]->PRINT = false;
      // modify parameters
      sims_[i]->cache_.behavior_params->mainStrategy.maxArcAngle += diff;
      //sims_[i]->restart();
    }

    // run param test
    runParamTest();
  }
}


void BehaviorSimulation::runKickTests(){

  KickStrategy cfgDribbleStrategy = KickStrategy();
  cfgDribbleStrategy.edgeBuffer = 500;
  cfgDribbleStrategy.postAngle = 30 * DEG_T_RAD;
  cfgDribbleStrategy.forwardOpeningAngle = 50 * DEG_T_RAD;
  cfgDribbleStrategy.insidePostBuffer = 220;
  cfgDribbleStrategy.maxArcAngle = 30 * DEG_T_RAD;
  cfgDribbleStrategy.shootOnGoalRadius = 0;
  cfgDribbleStrategy.ownGoalRadius = 1500;
  cfgDribbleStrategy.maxOpponentSD = 850.0;
  cfgDribbleStrategy.minOpponentDist = 700;
  cfgDribbleStrategy.allowOpponentSideDist = -1;
  cfgDribbleStrategy.opponentWidth = 50;
  cfgDribbleStrategy.orientationErrorFactor = 2.5;
  cfgDribbleStrategy.passDistance = 850;
  cfgDribbleStrategy.maxKickAngle = 10.0 * DEG_T_RAD;
  //cfgDribbleStrategy.supportFwdDist = -800;
  //cfgDribbleStrategy.supportSideDist = 1200;
  //cfgDribbleStrategy.defendBackDist = 1800;
  //cfgDribbleStrategy.forwardFwdDist = 1800;
  //cfgDribbleStrategy.forwardSideDist = 700;
  cfgDribbleStrategy.minOppDistForSlowKick = 7500;
  for (int i = 0; i < NUM_KICKS; i++){
    cfgDribbleStrategy.setUsableKick(i,false);
  }
  cfgDribbleStrategy.setUsableKick(Dribble,true);


  for (int test = 12; test < 13; test++){
    cout << "TEST " << test << endl;

    // set all red players with this behavior
    for (int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++){
      cout << "i is " << i << endl;
      if(sims_[i] == nullptr) continue;
      cout << " and setting " << i << endl;
      sims_[i]->PRINT = false;

      // set equal to blue kick strategy
      sims_[i]->cache_.behavior_params->mainStrategy = sims_[1]->cache_.behavior_params->mainStrategy;
      sims_[i]->cache_.behavior_params->clusterStrategy = sims_[1]->cache_.behavior_params->clusterStrategy;


      switch(test){

        // first... compare cluster strategies
        // 0 side kick cluster strategy
      case 0:
        {
          cout << endl << "Test " << test << ": Use side kick cluster strategy" << endl << endl;
          sims_[i]->cache_.behavior_params->clusterStrategy.behavior = Cluster::SIDEKICK;
          break;
        }

        // 1 dribble cluster strategy
      case 1:
        {
          cout << endl << "Test " << test << ": Use dribble cluster strategy" << endl << endl;
          sims_[i]->cache_.behavior_params->clusterStrategy.behavior = Cluster::DRIBBLE;
          break;
        }

        // 2 no cluster strategy
      case 2:
        {
          cout << endl << "Test " << test << ": Use no cluster strategy" << endl << endl;
          sims_[i]->cache_.behavior_params->clusterStrategy.behavior = Cluster::NONE;
          break;
        }

        // next.. try turning on different kicks

        // 3 enable all angle kicks
      case 3:
        {
          cout << endl << "Test " << test << ": Use all angle kicks (med, long, goal, pass)" << endl << endl;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongLargeGapKick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongSmallGapKick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdPass4Kick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdPass3Kick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdPass2Kick] = true;
          break;
        }

        // 4 enable just goal gap kicks
      case 4:
        {
          cout << endl << "Test " << test << ": Use goal gap kicks" << endl << endl;

          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongLargeGapKick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongSmallGapKick] = true;
          break;
        }

        // 5 enable all angle kicks except passes
      case 5:
        {
          cout << endl << "Test " << test << ": Use all angle kicks except passes (fwd, med, goal)" << endl << endl;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongLargeGapKick] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdLongSmallGapKick] = true;
          break;
        }

        // 6 enable short kicks
      case 6:
        {
          cout << endl << "Test " << test << ": Use short fwd kick" << endl << endl;

          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdShortStraightKick] = true;
          break;
        }

        // 9 try just dribbling
      case 9:
        {
          cout << endl << "Test " << test << ": Use only dribble strategy" << endl << endl;

          sims_[i]->cache_.behavior_params->mainStrategy = cfgDribbleStrategy;
          break;
        }
        // 8 enable side kicks normally
      case 8:
        {
          cout << endl << "Test " << test << ": Use fast side kicks in normal strategy" << endl << endl;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[WalkKickLeftwardSide] = true;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[WalkKickRightwardSide] = true;
          break;
        }
        // 7 change kick distances
      case 7:
        {
          cout << endl << "Test " << test << ": Use add kick between med and long" << endl << endl;

          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[FwdShortStraightKick] = true;

          break;
        }
        // no quick angle kicks
      case 10:
        {
          cout << endl << "Test " << test << ": No quick angle kicks" << endl << endl;

          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[WalkKickLeftward] = false;
          sims_[i]->cache_.behavior_params->mainStrategy.usableKicks[WalkKickRightward] = false;


          break;
        }
        // case 11, change kick ordering for red team..
      case 11:
        {
          cout << "TEST 11: rank fast kicks higher for red" << endl;
          /*
          WalkKickFront -= 13;
          WalkKickLeftward -= 13;
          WalkKickRightward -= 13;
          WalkKickLeftwardSide -= 13;
          WalkKickRightwardSide -= 13;
          FwdMediumStraightKick += 5;
          FwdMediumLeftwardKick += 5;
          FwdMediumRightwardKick += 5;
          FwdPass4Kick += 5;
          FwdPass3Kick += 5;
          FwdPass2Kick += 5;
          FwdShortStraightKick += 5;
          FwdShortLeftwardKick += 5;
          FwdShortRightwardKick += 5;
          Dribble += 5;
          LeftwardSideKick += 5;
          RightwardSideKick += 5;
          FastKick += 5;
          */
          break;
        }
      case 12:
        {
          cout << "Player " << i << " setting opponent Width to 50" << endl;
          sims_[i]->cache_.behavior_params->mainStrategy.opponentWidth = 50;
          break;
        }
          
      default:
        {
          cout << "ERROR test " << test << endl;
          break;
        }
      } // switch

    }


    // run param test
    runParamTest();

  }
}


void BehaviorSimulation::runParamTest(){

  simBlueScore = 0;
  simRedScore = 0;
  simPenaltyKick =false;
  numHalves = 0;
  PRINT = false;
  simTimer = 45;
  halfTimer = 655;
  changeSimulationState(INITIAL);
  gtcache.game_state->setState(INITIAL);
  simTimer = 5.0;
  while (numHalves < 80)
    simulationStep();
}

void BehaviorSimulation::restart() {
  for(auto& s : sims_)
    if(s != nullptr)
      s->core->interpreter_->restart();
}

MemoryCache BehaviorSimulation::getGtMemoryCache(int player) const {
  if(player == 0) {
    return MemoryCache(memory_.get());
  }
  if(sims_[player] == nullptr) {
    std::cerr << "Invalid player memory cache requested: " << player << "\n";
    return MemoryCache();
  }
  return sims_[player]->getMemoryCache();
}

MemoryCache BehaviorSimulation::getBeliefMemoryCache(int player) const {
  if(player == 0 && currentSim > 0) return sims_[currentSim]->getMemoryCache();
  else return getGtMemoryCache(player);
}

bool BehaviorSimulation::complete() {
  return gtcache.game_state->secsRemaining <= 0;
}

int BehaviorSimulation::defaultPlayer() const {
  int player = 0;
  for(int i = 0; i < sims_.size(); i++) {
    if(sims_[i] != nullptr) {
      player = i; break;
    }
  }
  return 0;
}

std::vector<int> BehaviorSimulation::activePlayers() const {
  std::vector<int> active;
  for(int i = 0; i < sims_.size(); i++) {
    if(sims_[i] != nullptr) {
      active.push_back(i);
    }
  }
  return active;

}
