#include <tool/simulation/ObservationGenerator.h>
#include <memory/WorldObjectBlock.h>
#include <memory/JointBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/OpponentBlock.h>

#define MISSED_OBS_FACTOR 1.5
#define VISION_ERROR_FACTOR 1.0

#define getObject(gt,obs,idx) \
  auto& gt = gt_object_->objects_[idx]; \
  auto& obs = obs_object_->objects_[idx];

#define getSelf(gt,obs,idx) getObject(gt,obs,team_ == TEAM_RED ? idx + WO_TEAM_LAST : idx)

#define isVisible(idx) \
  (fabs(normalizeAngle(gt_object_->objects_[player_].loc.getBearingTo(gt_object_->objects_[idx].loc, gt_object_->objects_[player_].orientation + joint_->values_[HeadPan]))) < FOVx/2.0)

ObservationGenerator::ObservationGenerator() : iparams_(Camera::TOP) {
  obs_object_ = new WorldObjectBlock();
  obs_object_->init();
  gt_object_ = NULL;
  opponent_mem_ = NULL;
  frame_info_ = NULL;
  joint_ = NULL;
  lconfig_.load(util::cfgpath(util::Modules) + "/localization.yaml");
}

ObservationGenerator::~ObservationGenerator() {
  if(obs_object_) delete obs_object_;
}

ObservationGenerator::ObservationGenerator(const ObservationGenerator& og) : iparams_(Camera::TOP) {
  *this = og;
  obs_object_ = new WorldObjectBlock();
  gt_object_ = NULL;
  opponent_mem_ = NULL;
  frame_info_ = NULL;
  joint_ = NULL;
}

void ObservationGenerator::setPlayer(int player, int team) {
  player_ = player;
  team_ = team;
}

void ObservationGenerator::setObjectBlocks(WorldObjectBlock* gtObjects, WorldObjectBlock* obsObjects) {
  vector<WorldObjectBlock*> obs = { obsObjects };
  setObjectBlocks(gtObjects, obs);
}

void ObservationGenerator::setObjectBlocks(WorldObjectBlock* gtObjects, vector<WorldObjectBlock*> obsObjects) {
  gt_object_ = gtObjects;
  obs_objects_ = obsObjects;
}

void ObservationGenerator::setModelBlocks(OpponentBlock* opponentMem) {
  opponent_mem_ = opponentMem;
}

void ObservationGenerator::setInfoBlocks(FrameInfoBlock* frameInfo, JointBlock* joint) {
  frame_info_ = frameInfo;
  joint_ = joint;
}

void ObservationGenerator::generateBallObservations() {
  getSelf(gtSelf,obsSelf,player_);
  getObject(gtBall,obsBall,WO_BALL);
  float bearing = gtSelf.loc.getBearingTo(gtBall.loc,gtSelf.orientation);
  float distance = gtSelf.loc.getDistanceTo(gtBall.loc);
  if (isVisible(WO_BALL)) {
    float maxDist = 5500.f;
    // Sigmoid shape
    float missedObsRate = min(1.f / (1.f + exp(-(distance/maxDist-.5f)*10.f)), 1.f);
    float randPct = Random::inst().sampleU();
    bool visible = true;
    if(distance > maxDist) visible = false;
    // check for other robots obstructing us
    for (int j = WO_PLAYERS_FIRST; j <= WO_PLAYERS_LAST; j++){
      if (player_ == j) continue;
      auto& gtOther = gt_object_->objects_[j];
      float otherBearing = gtSelf.loc.getBearingTo(gtOther.loc,gtSelf.orientation);
      float otherDistance = gtSelf.loc.getDistanceTo(gtOther.loc);
      if (otherDistance < distance && fabs(normalizeAngle(otherBearing - bearing)) < 4.0*DEG_T_RAD){
        visible = false;
        break;
      }
    }
    gtBall.relVel = gtBall.absVel;
    gtBall.relVel.rotate(-gtSelf.orientation);
    if(fabs(gtBall.relVel.y) > 2000)
      randPct *= 1.0f/8;
    if (visible && randPct > missedObsRate) {
      // seen
      obsBall.seen = true;
      obsBall.visionConfidence = 1.0;
      obsBall.frameLastSeen = frame_info_->frame_id;
      float diff = joint_->values_[HeadPan] - bearing;
      obsBall.imageCenterX = iparams_.width/2.0 + (diff / (FOVx/2.0) * iparams_.width/2.0);
      obsBall.imageCenterY = iparams_.height/2.0;
      float randNoise = Random::inst().sampleU() - .5;
      obsBall.visionDistance = distance + randNoise * VISION_ERROR_FACTOR * 0.05*distance;
      obsBall.visionBearing = bearing + randNoise * VISION_ERROR_FACTOR * 5.0*DEG_T_RAD;
    }
  }
}

void ObservationGenerator::generateLineObservations() {}

void ObservationGenerator::generateOpponentObservations() {}

void ObservationGenerator::generateCenterCircleObservations() {}

void ObservationGenerator::generateBeaconObservations() {
  getSelf(gtSelf,obsSelf,player_);
  std::vector<WorldObjectType> types = {
    WO_BEACON_BLUE_YELLOW,
    WO_BEACON_YELLOW_BLUE,
    WO_BEACON_BLUE_PINK,
    WO_BEACON_PINK_BLUE,
    WO_BEACON_PINK_YELLOW,
    WO_BEACON_YELLOW_PINK
  };
  for(auto t : types) {
    getObject(gtBeacon, obsBeacon, t);
    float bearing = gtSelf.loc.getBearingTo(gtBeacon.loc,gtSelf.orientation);
    float distance = gtSelf.loc.getDistanceTo(gtBeacon.loc);
    if (isVisible(t)) {
      // Rate of expected misses
      float missedObsRate = 1.0/5.0;
      float randPct = Random::inst().sampleU();
      // Only allow beacons to be seen up to 3 meters away
      if (randPct > (missedObsRate * MISSED_OBS_FACTOR) && distance < 3000){
        obsBeacon.seen = true;
        float diff = joint_->values_[HeadPan] - bearing;
        obsBeacon.imageCenterX = iparams_.width/2.0 + (diff / (FOVx/2.0) * iparams_.width/2.0);
        obsBeacon.imageCenterY = iparams_.height/2.0;
        // Add distance and bearing noise
        float randNoise = Random::inst().sampleU()-0.5;
        obsBeacon.visionDistance = distance + randNoise * VISION_ERROR_FACTOR * 0.2*distance;// up to 15% distance error
        obsBeacon.visionBearing = bearing + randNoise * VISION_ERROR_FACTOR * 10.0*DEG_T_RAD;// up to 5 deg bearing error
        obsBeacon.visionConfidence = 1.0;
      }
    }
  }
}

void ObservationGenerator::generateGoalObservations() {
  getSelf(gtSelf, obsSelf, player_);
  int seenPostCount = 0;
  int firstPost = 0;
  for (int i = WO_OWN_LEFT_GOALPOST; i <= WO_OPP_RIGHT_GOALPOST; i++){
    getObject(gtPost, obsPost, i);
    float bearing = gtSelf.loc.getBearingTo(gtPost.loc,gtSelf.orientation);
    float distance = gtSelf.loc.getDistanceTo(gtPost.loc);
    if(isVisible(i)) {
      float missedObsRate = 1.0/10.0;
      if (distance > 3000) 
        missedObsRate = 1.0/3.0;
      float randPct = Random::inst().sampleU();
      if (randPct > (missedObsRate * MISSED_OBS_FACTOR) && distance < 7000){
        // seen
        obsPost.seen = true;
        if (seenPostCount == 0) firstPost = i;
        seenPostCount++;
        float diff = joint_->values_[HeadPan] - bearing;
        obsPost.imageCenterX = iparams_.width/2.0 + (diff / (FOVx/2.0) * iparams_.width/2.0);
        obsPost.imageCenterY = iparams_.height/2.0;
        // add distance and bearing noise
        float randNoise = Random::inst().sampleU()-0.5;
        obsPost.visionDistance = distance + randNoise * VISION_ERROR_FACTOR * 0.25*distance;// up to 25% distance error
        obsPost.visionBearing = bearing + randNoise * VISION_ERROR_FACTOR * 5.0*DEG_T_RAD;// up to 5 deg bearing error
        obsPost.visionConfidence = 1.0;


      }
    }
  }

  // actually have to fill those into unknown post spot
  if (seenPostCount == 1){
    // fill in unknown post
    WorldObject& obsPost = obs_object_->objects_[WO_UNKNOWN_GOALPOST];
    WorldObject& known = obs_object_->objects_[firstPost];
    obsPost.seen = true;
    obsPost.frameLastSeen = frame_info_->frame_id;
    obsPost.imageCenterX = known.imageCenterX;
    obsPost.imageCenterY = known.imageCenterY;
    obsPost.visionDistance = known.visionDistance;
    obsPost.visionBearing = known.visionBearing;
    obsPost.visionConfidence = 1.0;
    known.seen = false;

  }
  else if (seenPostCount == 2){
    // fill in left and right post and goal with average of two
    float sumX = 0;
    float sumDist = 0;
    float sumBear = 0;
    for (int i = 0; i < 2; i++){
      WorldObject& obsPost = obs_object_->objects_[WO_UNKNOWN_LEFT_GOALPOST+i];
      WorldObject& known = obs_object_->objects_[firstPost+(i*2)];
      obsPost.seen = true;
      obsPost.frameLastSeen = frame_info_->frame_id;
      obsPost.imageCenterX = known.imageCenterX;
      obsPost.imageCenterY = known.imageCenterY;
      obsPost.visionDistance = known.visionDistance;
      obsPost.visionBearing = known.visionBearing;
      obsPost.visionConfidence = 1.0;

      known.seen = false;
      sumX += obsPost.imageCenterX;
      sumDist += obsPost.visionDistance;
      sumBear += obsPost.visionBearing;

    }
    WorldObject& obsPost = obs_object_->objects_[WO_UNKNOWN_GOAL];
    obsPost.seen = true;
    obsPost.frameLastSeen = frame_info_->frame_id;
    obsPost.imageCenterX = sumX / 2.0;
    obsPost.imageCenterY = iparams_.height/2.0;
    obsPost.visionDistance = sumDist / 2.0;
    obsPost.visionBearing = sumBear / 2.0;
    obsPost.visionConfidence = 1.0;

  } else if (seenPostCount > 2){
    //cout << index_ << " error saw more than 2 posts: " << seenPostCount << endl;
  }
}

void ObservationGenerator::generateAllObservations() {
  obs_object_->reset();
  initializeBelief();
  generateBallObservations();
  // generateLineObservations();
  // generateOpponentObservations();
  // generateCenterCircleObservations();
  generateBeaconObservations();
  generateGoalObservations();
  // generatePenaltyCrossObservations();
  fillObservationObjects();
}

void ObservationGenerator::generateGroundTruthObservations(){
  initializeBelief();
  obs_object_->reset();

  for (int i = 1; i <= WO_OPPONENT_LAST; i++){
    OpponentModel
      &cmodel = opponent_mem_->locModels[i - WO_OPPONENT_FIRST],
      &pmodel = opponent_mem_->locModels[i - 1];
    if (team_ == TEAM_BLUE){
      obs_object_->objects_[i].loc = gt_object_->objects_[i].loc;
      obs_object_->objects_[i].orientation = gt_object_->objects_[i].orientation;
      obs_object_->objects_[i].absVel = gt_object_->objects_[i].absVel;
      if (i >= WO_OPPONENT_FIRST){
        // fill in opponent mem
        cmodel.alpha = 1;
        cmodel.X00 = gt_object_->objects_[i].loc.x / 10.0;
        cmodel.X10 = gt_object_->objects_[i].loc.y / 10.0;
        cmodel.P00 = cmodel.P11 = 10.0;
      }
    } else {
      // have to swap players and opponent indices for red team
      if (i == 0){
        obs_object_->objects_[i].loc = -gt_object_->objects_[i].loc;
        obs_object_->objects_[i].orientation = normalizeAngle(gt_object_->objects_[i].orientation + M_PI);
        obs_object_->objects_[i].absVel = -gt_object_->objects_[i].absVel;
      } else if (i <= WO_TEAM_LAST){
        obs_object_->objects_[i+WO_TEAM_LAST].loc = -gt_object_->objects_[i].loc;
        obs_object_->objects_[i+WO_TEAM_LAST].orientation = normalizeAngle(gt_object_->objects_[i].orientation + M_PI);
        obs_object_->objects_[i+WO_TEAM_LAST].absVel = -gt_object_->objects_[i].absVel;
        // fill in opponent mem
        pmodel.alpha = 1.0;
        pmodel.X00 = -gt_object_->objects_[i].loc.x / 10.0;
        pmodel.X10 = -gt_object_->objects_[i].loc.y / 10.0;
        pmodel.P00 = pmodel.P11 = 10.0;
      } else {
        obs_object_->objects_[i-WO_TEAM_LAST].loc = -gt_object_->objects_[i].loc;
        obs_object_->objects_[i-WO_TEAM_LAST].orientation = normalizeAngle(gt_object_->objects_[i].orientation + M_PI);
        obs_object_->objects_[i-WO_TEAM_LAST].absVel = -gt_object_->objects_[i].absVel;
      }
    }
  } // copy all players and ball into our memory

  opponent_mem_->syncModels();
  WorldObject* oball = &(obs_object_->objects_[WO_BALL]);
  WorldObject* orobot = &(obs_object_->objects_[player_]);
  auto gtball = gt_object_->objects_[WO_BALL];
  auto gtrobot = gt_object_->objects_[player_];
  if(team_ == TEAM_RED) {
    gtrobot.loc *= -1;
    gtrobot.orientation = normalizeAngle(gtrobot.orientation + M_PI);
    gtball.loc *= -1;
    gtball.orientation = normalizeAngle(gtball.orientation + M_PI);
    gtball.absVel *= -1;
  }
  orobot->sdOrientation = 25.0*DEG_T_RAD;

  // update ball relative velocity from absolute
  // have to make a copy first because rotate rotates the actual point

  for (int i = 0; i < NUM_WORLD_OBJS; i++){
    if (i == WO_ROBOT_CLUSTER)
      continue;
    WorldObject* wo = &(obs_object_->objects_[i]);
    WorldObject* gto = &gt_object_->objects_[i];
    // calculate distance and bearing to each object
    auto distance = gtrobot.loc.getDistanceTo(gt_object_->objects_[i].loc);
    auto bearing = gtrobot.loc.getBearingTo(gt_object_->objects_[i].loc, gtrobot.orientation);

    // decide if seen depending on pan
    if (fabs(joint_->values_[HeadPan] - bearing) < FOVx/2.0 && distance < 5'000){
      if(wo->isUnknown()) continue;
      gto->seen = wo->seen = true;
      gto->frameLastSeen = wo->frameLastSeen = frame_info_->frame_id;
      float diff = joint_->values_[HeadPan] - wo->bearing;
      wo->imageCenterX = iparams_.width/2.0 + (diff / (FOVx/2.0) * iparams_.width/2.0);
      wo->imageCenterY = iparams_.height/2.0;
      gto->visionDistance = wo->visionDistance = wo->distance;
      gto->visionBearing = wo->visionBearing = wo->bearing;
    } else gto->seen = wo->seen = false;
  }
  fillObservationObjects();
}

void ObservationGenerator::generatePenaltyCrossObservations() {}

void ObservationGenerator::initializeBelief() {
  if(initialized_) return;
  //TODO: obs_objects_ should really just be one cache, 
  //use a different generator for different ones or something
  //because these updates don't make sense with multiple observation caches.
  for(auto obs : obs_objects_) {
    for(int i = 0; i < NUM_WORLD_OBJS; i++) {
      auto& from = obs->objects_[i];
      auto& to = obs_object_->objects_[i];
      to.seen = from.seen;
      to.frameLastSeen = from.frameLastSeen;
      to.imageCenterX = from.imageCenterX;
      to.imageCenterY = from.imageCenterY;
      to.visionDistance = from.visionDistance;
      to.visionBearing = from.visionBearing;
      to.visionPt1 = from.visionPt1;
      to.visionPt2 = from.visionPt2;
      to.visionLine = from.visionLine;
      to.distance = from.distance;
      to.bearing = from.bearing;
      to.loc = from.loc;
      to.orientation = from.orientation;
      to.relPos = from.relPos;
      to.relVel = from.relVel;
      to.absVel = from.absVel;
      to.sdOrientation = from.sdOrientation;
    }
    // These updates assume that loc is given correctly and nothing else really is,
    // which conforms to the FieldConfiguration class only setting position (loc)
    // and orientation
    auto& self = obs->objects_[player_];
    for(int i = 0; i < NUM_WORLD_OBJS; i++) {
      auto& obj = obs_object_->objects_[i];
      obj.distance = (self.loc - obj.loc).getMagnitude();
      obj.bearing = self.loc.getBearingTo(obj.loc, self.orientation);
      obj.relPos = obj.loc.globalToRelative(self.loc, self.orientation);
      obj.visionDistance = obj.distance;
      obj.visionBearing = obj.bearing;
      obj.frameLastSeen = -10000; // make this a long time ago so that we think it's lost
    }
  }
  initialized_ = true;
}

void ObservationGenerator::fillObservationObjects() {
  for(auto obs : obs_objects_) {
    for(int i = 0; i < NUM_WORLD_OBJS; i++) {
      auto& from = obs_object_->objects_[i];
      auto& to = obs->objects_[i];
      to.seen = from.seen;
      to.frameLastSeen = from.frameLastSeen;
      to.imageCenterX = from.imageCenterX;
      to.imageCenterY = from.imageCenterY;
      to.visionDistance = from.visionDistance;
      to.visionBearing = from.visionBearing;
      to.visionPt1 = from.visionPt1;
      to.visionPt2 = from.visionPt2;
      to.visionLine = from.visionLine;
      to.distance = from.distance;
      to.bearing = from.bearing;
      to.loc = from.loc;
      to.orientation = from.orientation;
      to.relPos = from.relPos;
      to.relVel = from.relVel;
      to.absVel = from.absVel;
      to.sdOrientation = from.sdOrientation;
    }
  }
}
