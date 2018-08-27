#include <tool/simulation/IsolatedBehaviorSimulation.h>
#include <localization/LocalizationModule.h>
#include <localization/LocalizationState.h>
#include <common/FieldConfiguration.h>
#include <tool/UTMainWnd.h>

#define SECONDS_PER_FRAME (1.0/30.0)
#define RADS_PER_FRAME (DEG_T_RAD * 1.0)
#define DISTANCE_PER_FRAME 5.0

#define getObject(obj,idx) \
  auto& obj = gtcache_.world_object->objects_[idx];

#define TRANSITION(state) { approachState_ = state; stateTime_ = 0; }

#define team_ 0

typedef IsolatedBehaviorSimulation IBSim;
typedef ObjectConfiguration OP;

IBSim::IsolatedBehaviorSimulation(bool locMode, int player) : 
    lmode_(locMode),
    player_(player),
    sim_(team_, player, locMode),
    cg_(team_, player),
    iparams_(Camera::TOP) {
  gtcache_ = MemoryCache::create(team_, player_);
  bcache_ = sim_.getMemoryCache();
  loadConfig();
  cg_.setCaches(gtcache_, bcache_);
  bcache_.robot_state->role_ = CHASER;
  gtcache_.game_state->setState(PLAYING);
  bcache_.game_state->setState(PLAYING);
  sim_.initLocalization();
  activePlayers_ = { player_ };
  physics_.setObjects(gtcache_.world_object);
  approachState_ = LONG_RANGE_APPROACH;
  stateTime_ = 0;
}

void IBSim::randomizePlayers(FieldConfiguration& config) {
  rand_ = Random(time(NULL));
  for(int i = WO_TEAM1; i <= WO_OPPONENT5; i++) {
    config[i].loc.x = rand_.sampleU(0.f, HALF_FIELD_X);
    config[i].loc.y = rand_.sampleU(-1500, 1500);
  }
  config[player_].loc.x = rand_.sampleU(0.f, HALF_FIELD_X);
  config[player_].loc.y = rand_.sampleU(-HALF_FIELD_Y, HALF_FIELD_Y);
  config[player_].orientation = rand_.sampleU(-M_PI,M_PI);
  
  config[WO_BALL].loc.x = rand_.sampleU(0.f, HALF_FIELD_X);
  config[WO_BALL].loc.y = rand_.sampleU(-HALF_FIELD_Y, HALF_FIELD_Y);
}

void IBSim::loadConfig() {
  auto 
    &gtconfig = config_.objects.gtconfig,
    &bconfig = config_.objects.bconfig;
  if(!loadGame("iso_behavior_sim")) {
    gtconfig = { 
      {player_, OP(-750, 0, 0)},
      {WO_BALL, OP(0, 0)}
    };
    bconfig = {
      {player_, OP(-750, 0, 0)},
      {WO_BALL, OP(0, 0)}
    };
  }
  randomizePlayers(gtconfig);
  randomizePlayers(bconfig);
  saveGame("iso_behavior_sim");
  applyConfig(gtcache_, {bcache_});
}

void IBSim::moveBallRandomly() {
  getObject(ball, WO_BALL);
  getObject(self, gtcache_.robot_state->WO_SELF);
  float dist = ball.loc.getDistanceTo(self.loc);
  stateTime_++;
  if(approachState_ == LONG_RANGE_APPROACH && dist < 200)
    TRANSITION(SHORT_RANGE_APPROACH)
  else if(approachState_ == SHORT_RANGE_APPROACH && stateTime_ > 100)
    TRANSITION(SHORT_RANGE_COMPLETE)
  else if(approachState_ == SHORT_RANGE_WAIT && stateTime_ > 100)
    TRANSITION(LONG_RANGE_APPROACH)
  if(approachState_ == SHORT_RANGE_COMPLETE) {
    TRANSITION(SHORT_RANGE_WAIT);
    int xquad, yquad;
    xquad = ball.loc.x > 0 ? -1 : 1;
    yquad = ball.loc.y > 0 ? -1 : 1;
    Point2D target(
        rand_.sampleU(min(xquad * FIELD_X/2, 0.0f), max(xquad * FIELD_X/2, 0.0f)), 
        rand_.sampleU(min(yquad * FIELD_Y/2, 0.0f), max(yquad * FIELD_Y/2, 0.0f))
    );
    physics_.moveBall(target);

    //// Simulate losing the ball
    if(!lmode()) {
      ball.sd.x = ball.sd.y = 10000;
      bcache_.frame_info->seconds_since_start += 2.0;
    }
  }
}

void IBSim::simulationStep() {
  physics_.step();
  //moveBallRandomly();
  gtcache_.frame_info->frame_id++;
  for(int i = WO_TEAM1; i <= WO_TEAM5; i++) {
    if(i == player_) continue;
    gtcache_.team_packets->frameReceived[i] = gtcache_.frame_info->frame_id;
    gtcache_.team_packets->relayData[i].bvrData.state == PENALISED;
    bcache_.team_packets->frameReceived[i] = gtcache_.frame_info->frame_id;
    bcache_.team_packets->relayData[i].bvrData.state == PLAYING;
  }
  //cg_.generateAllCommunications();
  sim_.processFrame(gtcache_.world_object, gtcache_.game_state);
}

void IBSim::teleportPlayer(Point2D position, float orientation, int player) {
  if(find(activePlayers_.begin(), activePlayers_.end(), player) == activePlayers_.end()) return;
  auto& woPlayer = gtcache_.world_object->objects_[player];
  woPlayer.loc = position;
  woPlayer.orientation = orientation;
}

void IBSim::movePlayer(Point2D position, float orientation, int player) {
  if(find(activePlayers_.begin(), activePlayers_.end(), player) == activePlayers_.end()) return;
  auto& gtPlayer = gtcache_.world_object->objects_[player];
  gtPlayer.loc = position;
  gtPlayer.orientation = orientation;
  auto& bPlayer = bcache_.world_object->objects_[player];
  bPlayer.loc = position;
  bPlayer.orientation = orientation;
  if(lmode_) {
    bcache_.localization_mem->state[0] = position.x;
    bcache_.localization_mem->state[1] = position.y;
    sim_.core->localization_->initFromMemory();
  }
}

void IBSim::teleportBall(Point2D pos) {
  TRANSITION(LONG_RANGE_APPROACH);
  auto& woBall = gtcache_.world_object->objects_[WO_BALL];
  woBall.loc = pos;
  woBall.absVel = Point2D(0,0);
}

void IBSim::moveBall(Point2D target) {
  TRANSITION(LONG_RANGE_APPROACH);
  physics_.moveBall(target);
}

vector<string> IBSim::getTextDebug(int player) {
  return sim_.getTextDebug();
}
