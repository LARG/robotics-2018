#include "OppModule.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <opponents/Logging.h>

void OppModule::specifyMemoryDependency() {
  requiresMemoryBlock("world_objects");
  requiresMemoryBlock("opponents");
  requiresMemoryBlock("team_packets");
  requiresMemoryBlock("vision_frame_info");
  requiresMemoryBlock("robot_state");
  requiresMemoryBlock("game_state");
  requiresMemoryBlock("vision_processed_sonar");
}

void OppModule::specifyMemoryBlocks() {
  getMemoryBlock(worldObjects,"world_objects");
  getOrAddMemoryBlock(opponentMem,"opponents");
  getOrAddMemoryBlock(teamPacketsMem,"team_packets");
  getMemoryBlock(frameInfo,"vision_frame_info");
  getMemoryBlock(robotState,"robot_state");
  getMemoryBlock(gameState,"game_state");
  getOrAddMemoryBlock(processedSonar,"vision_processed_sonar");

  for (int m = 0; m < c_MAX_MODELS; m++){
    models[m].setMemory(worldObjects, robotState, frameInfo);
  }
}

void OppModule::initSpecificModule(){
  cout << "OppModule init" << endl;

  timePassed=0.0;
  timeLast=0.0;

  models[0].isActive = false; // Start with no models
  models[0].toBeActivated = false;

  timeLast = frameInfo->seconds_since_start;

  for (int m = 0; m < c_MAX_MODELS; m++){
    models[m].setMemory(worldObjects, robotState, frameInfo);
    models[m].setTextLogger(textlogger);
  }
  tlog(70, "done setting indiv opp model memory");

}


void OppModule::reInit(){
  timeLast = frameInfo->seconds_since_start;

  for (int i = 0; i < c_MAX_MODELS; i++){
    models[i].isActive = false; // Start with just the first model
    models[i].toBeActivated = false;
  }

  SmallUKF4Params modelParams;
  modelParams.kappa = ukfParams.kappa;
  modelParams.vel_decay_rate = ukfParams.vel_decay_rate;
  modelParams.outlier_rejection_thresh = ukfParams.outlier_rejection_thresh;
  modelParams.robot_pos_noise = ukfParams.robot_pos_noise;
  modelParams.robot_vel_noise = ukfParams.robot_vel_noise;
  modelParams.init_sd_x = ukfParams.init_sd_x;
  modelParams.init_sd_y = ukfParams.init_sd_y;
  modelParams.init_sd_vel = ukfParams.init_sd_vel;

  for (int m = 0; m < c_MAX_MODELS; m++){
    models[m].setMemory(worldObjects, robotState, frameInfo);
    models[m].init();
    models[m].setParams(modelParams);
  }

  previousGameState = gameState->state();
  previousTeam = robotState->team_;

  doOpponentReset();

  tlog(70, "done setting opp model python params");
}


//--------------------------------- MAIN FUNCTIONS  ---------------------------------//


void OppModule::processFrame(){
  if (!isMemorySatisfied()) return;

  tlog(30, "oppLoc processFrame start %5.3f, nModels: %i", frameInfo->seconds_since_start, getNumActiveModels());

  // convert wo to cm for ukf
  mmTocm();

  // we know where opponents are in PK other than in playing
  if (gameState->isPenaltyKick && gameState->state() != PLAYING){
    doPenaltyKickReset();
  }
  if (previousGameState != gameState->state() && previousGameState == INITIAL && !gameState->isPenaltyKick){
    tlog(10, "Changed state from initial, do player reset");
    doOpponentReset();
  }
  // also if we changed teams
  else if (previousTeam != robotState->team_){
    tlog(10, "Team changed, do player reset");
    doOpponentReset();
  }

  // compute time from last frame (for odom update)
  timePassed = frameInfo->seconds_since_start - timeLast;
  timeLast = frameInfo->seconds_since_start;

  runTimeUpdates();

  // updates from observations
  if (ukfParams.USE_TEAM)
    checkOpponentTeamPackets();

  // none of this while penalised
  if (gameState->state() != PENALISED){
    if (ukfParams.USE_VISION)
      doOpponentVisionUpdate();
    
    if (ukfParams.USE_SONAR)
      doOpponentSonarUpdate();
    
    if (ukfParams.USE_BUMP){
      if (processedSonar->bump_left_)
        doOpponentBumpUpdate(M_PI/2.0);
      if (processedSonar->bump_right_)
        doOpponentBumpUpdate(-M_PI/2.0);
    }
  }

  NormaliseAlphas();

  MergeModels(ukfParams.max_models_after_merge);

  clipActiveModelsToField();

  // update world objects
  populateOpponents();

  // update memory for debug
  updateMemory();

  // convert wo back to mm
  cmTomm();

  previousGameState = gameState->state();
  previousTeam = robotState->team_;

  tlog(30, "oppLoc processFrame complete %5.3f, nModels: %i", frameInfo->seconds_since_start, getNumActiveModels());
}

void OppModule::doOpponentReset() {

  // Todd: by default, assume there is a keeper
  tlog(30,"opp filter opponent reset - assume keeper");

  for (int i = 0; i < c_MAX_MODELS; i++){
    models[i].isActive = false; // Start with just the first model
    models[i].toBeActivated = false;
    models[i].alpha = 0.0;
  }

  models[0].isActive = true;
  models[0].alpha = 1.0;

  models[0].stateEstimates[0][0] = FIELD_X/20.0 -10.0;
  models[0].stateEstimates[1][0] = 0;
  models[0].stateEstimates[2][0] = 0;
  models[0].stateEstimates[3][0] = 0;

  models[0].stateStandardDeviations[0][0] = ukfParams.init_sd_x;
  models[0].stateStandardDeviations[1][1] = ukfParams.init_sd_y;
  models[0].stateStandardDeviations[2][2] = ukfParams.init_sd_vel;
  models[0].stateStandardDeviations[3][3] = ukfParams.init_sd_vel;

}

void OppModule::doPenaltyKickReset(){
  // only 1 opponent
  ukfParams.max_models_after_merge = 1;

  tlog(30,"opp filter penalty kick reset");

  for (int i = 0; i < c_MAX_MODELS; i++){
    models[i].isActive = false; // Start with just the first model
    models[i].toBeActivated = false;
    models[i].alpha = 0.0;
  }

  models[0].isActive = true;
  models[0].alpha = 1.0;

  models[0].stateEstimates[0][0] = 0;
  models[0].stateEstimates[1][0] = 0;
  models[0].stateEstimates[2][0] = 0;
  models[0].stateEstimates[3][0] = 0;

  models[0].stateStandardDeviations[0][0] = ukfParams.init_sd_x/100.0;
  models[0].stateStandardDeviations[1][1] = ukfParams.init_sd_y/100.0;
  models[0].stateStandardDeviations[2][2] = ukfParams.init_sd_vel/100.0;
  models[0].stateStandardDeviations[3][3] = ukfParams.init_sd_vel/100.0;

  if (robotState->WO_SELF == KEEPER){
    // opponent at 0,0
    models[0].stateEstimates[0][0] = 0;
  } else {
    // opponent in goal center
    models[0].stateEstimates[0][0] = FIELD_X/20.0 -10.0;
  }
}

void OppModule::resetSdMatrix(int modelNumber)
{
  // Set the uncertainties
  models[modelNumber].stateStandardDeviations[0][0] = ukfParams.init_sd_x;
  models[modelNumber].stateStandardDeviations[1][1] = ukfParams.init_sd_y;
  models[modelNumber].stateStandardDeviations[2][2] = ukfParams.init_sd_vel;
  models[modelNumber].stateStandardDeviations[3][3] = ukfParams.init_sd_vel;
  return;
}

bool OppModule::clipModelToField(int modelID)
{

  const float fieldXLength = GRASS_X/10.0;
  const float fieldYLength = GRASS_Y/10.0;
  const float fieldXMax = fieldXLength / 2.0;
  const float fieldXMin = - fieldXLength / 2.0;
  const float fieldYMax = fieldYLength / 2.0;
  const float fieldYMin = - fieldYLength / 2.0;

  bool wasClipped = false;
  bool clipped;
  float prevX, prevY;
  prevX = models[modelID].getState(0);
  prevY = models[modelID].getState(1);

  clipped = models[modelID].clipState(0, fieldXMin, fieldXMax);   // Clipping for robot's X
  if(clipped){
    tlog(20, "Model %i, Alpha: %5.3f, Clipped 0, prev: (%5.1f, %5.1f), new: ((%5.1f, %5.1f) ", modelID, models[modelID].alpha, prevX, prevY, models[modelID].getState(0), models[modelID].getState(1));
  }

  wasClipped = wasClipped || clipped;

  prevX = models[modelID].getState(0);
  prevY = models[modelID].getState(1);

  clipped = models[modelID].clipState(1, fieldYMin, fieldYMax);   // Clipping for robot's Y

  if(clipped){
    tlog(20, "Model %i, Alpha: %5.3f, Clipped 1, prev: (%5.1f, %5.1f), new: ((%5.1f, %5.1f), ", modelID, models[modelID].alpha, prevX, prevY, models[modelID].getState(0), models[modelID].getState(1));
  }

  wasClipped = wasClipped || clipped;

  return wasClipped;
}



bool OppModule::clipActiveModelsToField()
{
  bool wasClipped = false;
  bool modelClipped = false;
  for(int modelID = 0; modelID < c_MAX_MODELS; modelID++){
    if(models[modelID].isActive == true){
      modelClipped = clipModelToField(modelID);
      wasClipped = wasClipped || modelClipped;
    }
  }
  return wasClipped;
}


void OppModule::checkOpponentTeamPackets(){

  // if we're doing run core in the tool, set the updated flag back to true
  // so we'll process the shared packets again
#ifdef TOOL
  for (int i = WO_TEAM_FIRST; i <= WO_TEAM_LAST; i++){
    if ((frameInfo->frame_id-1) == (unsigned)(teamPacketsMem->frameReceived[i])){
      teamPacketsMem->oppUpdated[i] = true;
    }
  }
#endif

  // see if we have any team packets we haven't updated yet
  for (int i = WO_TEAM_FIRST; i <= WO_TEAM_LAST; i++){
    if (i == robotState->WO_SELF) continue;
    if (teamPacketsMem->oppUpdated[i]){

      // only ready, set, playing
      if (teamPacketsMem->relayData[i].bvrData.state != READY && teamPacketsMem->relayData[i].bvrData.state != SET &&  teamPacketsMem->relayData[i].bvrData.state != PLAYING){
        tlog(10, "ignore message from robot %i in state %i", i, teamPacketsMem->relayData[i].bvrData.state);
        teamPacketsMem->oppUpdated[i] = false;
        continue;
      }

      //oppStruct* tp = &(teamPacketsMem->tp[i].oppData[i]);
      tlog(20, "Do teammate opp update from mate %i recv frame %i", i, teamPacketsMem->frameReceived[i]);

      // loop through each  possible opponents in pkt
      for (int j = 0; j < MAX_OPP_MODELS_IN_MEM; j++){
        auto tp = &(teamPacketsMem->oppData[i].opponents[j]);

        // check that packet is ok
        if (!tp->filled) continue;

        // ignore packets where we havent seen opponent in a while
        if (tp->framesMissed > 30){
          tlog(20, "Teammate last saw opponent %i frames ago, no update", tp->framesMissed);
          continue;
        }

        if (std::isnan(tp->x) || std::isnan(tp->y) || tp->sdx == 0 || tp->sdxy == 0 || tp->sdy == 0 || (frameInfo->frame_id-teamPacketsMem->frameReceived[i]) > 15){
          tlog(20, "Bad teammate opponent pkt, either nan, sd=0, or bad frame #");
          teamPacketsMem->oppUpdated[i] = false;
          continue;
        }
        float sdFactor = ukfParams.team_sd_factor;
        doSharedOpponentUpdate(tp->x, tp->y, sdFactor*tp->sdx, sdFactor*tp->sdxy, sdFactor*tp->sdy);
      }

      // set as updated now
      teamPacketsMem->oppUpdated[i] = false;
    }
  }
}

int OppModule::doOpponentVisionUpdate(){

  WorldObject* self = &(worldObjects->objects_[robotState->WO_SELF]);

  tlog(10, "update using self %i, loc (%5.3f, %5.3f), orient %5.3f, sd (%5.5f, %5.5f), sd orient %5.5f", robotState->WO_SELF, self->loc.x, self->loc.y, self->orientation, self->sd.x, self->sd.y, self->sdOrientation);

  // check for detected opponents
  for (int opp = WO_OPPONENT_FIRST; opp <= WO_OPPONENT_LAST; opp++) {
    WorldObject* wo = &(worldObjects->objects_[opp]);
    if (!wo->seen)
      continue;
    float oppAng = wo->visionBearing;
    float oppDist = wo->visionDistance;

    if (oppDist > ukfParams.ignore_vision_dist) continue;

    tlog(40, "Saw opponent at dist %5.3f, bearing %5.3f", oppDist, oppAng*RAD_T_DEG);

    // no models -> create one
    int numActiveModels = getNumActiveModels();
    if (numActiveModels == 0){
      tlog(10, "No active opponent models, creating one");
      bool oppOk = initNewModel(0, oppDist, oppAng);
      if (!oppOk) continue;
    }

    // attempt update on each model, check if any succeed
    int numSuccess = 0;
    float bestInnov = 1000.0;
    int bestModel = -1;
    for (int i = 0; i < c_MAX_MODELS; i++){
      if (models[i].isActive){
        tlog(20, "attempt update on model %i", i);
        int result = models[i].opponentDetection(oppDist, oppAng, ukfParams.R_vision_range_offset, ukfParams.R_vision_range_relative, ukfParams.R_vision_theta);
        if (models[i].lastInnov2 < bestInnov && result == UKF4_OK){
          bestInnov = models[i].lastInnov2;
          bestModel = i;
        }
        if (result == UKF4_OK){
          numSuccess++;
          tlog(20, "successfully updated model %i with opp vision observation", i);
        }
      }
    }

    tlog(10, "best model was %i with innov %5.3f", bestModel, bestInnov);

    // revert changes to all but the best model
    if (numSuccess > 1){
      for (int i = 0; i < c_MAX_MODELS; i++){
        if (models[i].isActive && i != bestModel){
          tlog(20, "revert model %i", i);
          models[i].revert();
        }
      }
    }


    // if they all outliered, create a new one
    if (numSuccess == 0){
      tlog(10, "All opponent models outliered on vision update, creating one");
      int modelID = FindNextFreeModel();
      bool oppOk = initNewModel(modelID, oppDist, oppAng);
      if (oppOk){
        models[modelID].opponentDetection(oppDist, oppAng, ukfParams.R_vision_range_offset, ukfParams.R_vision_range_relative, ukfParams.R_vision_theta);
      }
    }
  } // opp loop

  return 0;
}


int OppModule::doOpponentBumpUpdate(float heading){

  WorldObject* self = &(worldObjects->objects_[robotState->WO_SELF]);

  tlog(10, "update using self %i, loc (%5.3f, %5.3f), orient %5.3f, sd (%5.5f, %5.5f), sd orient %5.5f", robotState->WO_SELF, self->loc.x, self->loc.y, self->orientation, self->sd.x, self->sd.y, self->sdOrientation);

  float dist = ukfParams.bump_distance_estimate;

  tlog(40, "Bump opponent at dist %5.3f, bearing %5.3f", dist, heading*RAD_T_DEG);

  // no models -> create one
  int numActiveModels = getNumActiveModels();
  if (numActiveModels == 0){
    tlog(10, "No active opponent models, creating one");
    bool oppOk = initNewModel(0, dist, heading);
    if (!oppOk) return 0;
  }

  // attempt update on each model, check if any succeed
  int numSuccess = 0;
  float bestInnov = 1000.0;
  int bestModel = -1;
  for (int i = 0; i < c_MAX_MODELS; i++){
    if (models[i].isActive){
      int result = models[i].opponentDetection(dist, heading, ukfParams.R_bump_range, 0, ukfParams.R_bump_theta);
      if (models[i].lastInnov2 < bestInnov){
        bestInnov = models[i].lastInnov2;
        bestModel = i;
      }
      if (result == UKF4_OK){
        numSuccess++;
        tlog(20, "successfully updated model %i with opp bump observation", i);
      }
    }
  }

  tlog(10, "best model was %i with innov %5.3f", bestModel, bestInnov);

  // revert changes to all but the best model
  if (numSuccess > 1){
    for (int i = 0; i < c_MAX_MODELS; i++){
      if (models[i].isActive && i != bestModel){
        tlog(20, "revert model %i", i);
        models[i].revert();
      }
    }
  }


  // if they all outliered, create a new one
  if (numSuccess == 0){
    tlog(10, "All opponent models outliered on bump update, creating one");
    int modelID = FindNextFreeModel();
    bool oppOk = initNewModel(modelID, dist, heading);
    if (oppOk){
      models[modelID].opponentDetection(dist, heading, ukfParams.R_bump_range, 0, ukfParams.R_bump_theta);
    }
  }

  return 0;
}

int OppModule::doOpponentSonarUpdate(){
  // draw opponent from sonar
  for (int opp = 0; opp < 3; opp++) {

    float angle = 0, distance = 0;
    if (opp == 0) {
      if (!processedSonar->on_left_) {
        continue;
      } else {
        angle = M_PI / 4;
        distance = processedSonar->left_distance_;
      }
    }
    if (opp == 1) {
      if (!processedSonar->on_center_) {
        continue;
      } else {
        angle = 0;
        distance = processedSonar->center_distance_;
      }
    }
    if (opp == 2) {
      if (!processedSonar->on_right_) {
        continue;
      } else {
        angle = -M_PI / 4;
        distance = processedSonar->right_distance_;
      }
    }

    // Angle and distance to opponent
    float oppAng = angle;
    float oppDist = distance * 1000; // convert from meters to mm

    // convert to cm
    oppDist /= 10.0;

    if (oppDist > ukfParams.ignore_sonar_dist) continue;

    tlog(40, "Sonar opponent at dist %5.3f, bearing %5.3f", oppDist, oppAng*RAD_T_DEG);

    // no models -> create one
    int numActiveModels = getNumActiveModels();
    if (numActiveModels == 0){
      tlog(10, "No active opponent models, creating one");
      bool oppOk = initNewModel(0, oppDist, oppAng);
      if (!oppOk) continue;
    }

    // attempt update on each model, check if any succeed
    int numSuccess = 0;
    float bestInnov = 1000.0;
    int bestModel = -1;
    for (int j = 0; j < c_MAX_MODELS; j++){
      if (models[j].isActive){
        int result = models[j].opponentDetection(oppDist, oppAng, ukfParams.R_sonar_range_offset, ukfParams.R_sonar_range_relative, ukfParams.R_sonar_theta);
        if (models[j].lastInnov2 < bestInnov){
          bestInnov = models[j].lastInnov2;
          bestModel = j;
        }
        if (result == UKF4_OK){
          numSuccess++;
          tlog(20, "successfully updated model %i with sonar opp observation", j);
        }
      }
    }

    tlog(10, "best model was %i with innov %5.3f", bestModel, bestInnov);

    // revert changes to all but the best model
    if (numSuccess > 1){
      for (int i = 0; i < c_MAX_MODELS; i++){
        if (models[i].isActive && i != bestModel){
          tlog(20, "revert model %i", i);
          models[i].revert();
        }
      }
    }

    // if they all outliered, create a new one
    if (numSuccess == 0){
      tlog(10, "All opponent models outliered on sonar, creating one");
      int modelID = FindNextFreeModel();
      bool oppOk = initNewModel(modelID, oppDist, oppAng);
      if (oppOk){
        models[modelID].opponentDetection(oppDist, oppAng, ukfParams.R_sonar_range_offset, ukfParams.R_sonar_range_relative, ukfParams.R_sonar_theta);
      }
    }
  } // opp loop

  return 0;
}

bool OppModule::initNewModel(int modelID, float oppDist, float oppAng){
  // create a new model hypothesis
  // starting at the seen opponent location

  models[modelID].reInit();

  WorldObject* self = &(worldObjects->objects_[robotState->WO_SELF]);

  float oppX = oppDist * cos(oppAng + self->orientation) + self->loc.x;
  float oppY = oppDist * sin(oppAng + self->orientation) + self->loc.y;

  // if not on field, return now
  if (fabs(oppX) > (GRASS_X/20.0) || fabs(oppY) > (GRASS_Y/20.0)){
    tlog(10, "new opponent off grass, ignoring (%5.3f, %5.3f)", oppX, oppY);
    models[modelID].isActive = false;
    return false;
  }

  models[modelID].stateEstimates[0][0] = oppX;
  models[modelID].stateEstimates[1][0] = oppY;

  models[modelID].isActive = true;
  models[modelID].alpha = 1.0;

  return true;
}


/** Time update for all models */
void OppModule::runTimeUpdates(){

  for(int modelID = 0; modelID < c_MAX_MODELS; modelID++){
    if(models[modelID].isActive == false) continue; // Skip Inactive models.
    models[modelID].timeUpdate(timePassed);
  }
}


int OppModule::doSharedOpponentUpdate(float x, float y, float sdxx, float sdxy, float sdyy){

  tlog(20, "Shared Opponent Update. SharedOpp: (%5.1f, %5.1f) SRXX: %5.1f, SRXY %5.1f, SRYY %5.1f", x, y, sdxx, sdxy, sdyy);

  // no models -> create one
  int numActiveModels = getNumActiveModels();
  if (numActiveModels == 0){
    tlog(10, "No active opponent models, creating one");
    models[0].init();
    models[0].isActive = true;
    models[0].stateEstimates[0][0] = x;
    models[0].stateEstimates[1][0] = y;
  }

  // attempt update on each model, check if any succeed
  int numSuccess = 0;
  float bestInnov = 1000.0;
  int bestModel = -1;
  for (int i = 0; i < c_MAX_MODELS; i++){
    if (!models[i].isActive) continue;
    int result = models[i].linear2MeasurementUpdate(x, y, sdxx, sdxy, sdyy, 0, 1);
    if (models[i].lastInnov2 < bestInnov){
      bestInnov = models[i].lastInnov2;
      bestModel = i;
    }
    if (result == UKF4_OK){
      numSuccess++;
      tlog(20, "successfully updated model %i with shared opp observation", i);
    }
  }

  tlog(10, "best model was %i with innov %5.3f", bestModel, bestInnov);

  // revert changes to all but the best model
  if (numSuccess > 1){
    for (int i = 0; i < c_MAX_MODELS; i++){
      if (models[i].isActive && i != bestModel){
        tlog(20, "revert model %i", i);
        models[i].revert();
      }
    }
  }

  // if they all outliered, create a new one
  if (numSuccess == 0){
    tlog(10, "All opponent models outliered, creating one");
    int modelID = FindNextFreeModel();
    models[modelID].init();
    models[modelID].isActive = true;
    models[modelID].stateEstimates[0][0] = x;
    models[modelID].stateEstimates[1][0] = y;
    models[modelID].linear2MeasurementUpdate(x, y, sdxx, sdxy, sdyy, 0, 1);
  }

  return 0;
}


bool OppModule::MergeTwoModels(int index1, int index2)
{
  // Merges second model into first model, then disables second model.
  bool success = true;
  if(index1 == index2) success = false; // Don't merge the same model.
  //    if((model[index1].active == false) || (model[index2].active == false)) success = false; // Both models must be active.
  if(success == false){

    tlog(10, "Merge models %i and %i FAILED", index1, index2);

    return success;
  }

  // Merge alphas
  float alphaMerged = models[index1].alpha + models[index2].alpha;
  float alpha1 = models[index1].alpha / alphaMerged;
  float alpha2 = models[index2].alpha / alphaMerged;

  NMatrix xMerged; // Merge State matrix

  // If one model is much more correct than the other, use the correct states.
  // This prevents drifting from continuouse splitting and merging even when one model is much more likely.
  if(models[index1].alpha > 10*models[index2].alpha){
    xMerged = models[index1].stateEstimates;
  }
  else if (models[index2].alpha > 10*models[index1].alpha){
    xMerged = models[index2].stateEstimates;
  }
  else {
    xMerged = (alpha1 * models[index1].stateEstimates + alpha1 * models[index2].stateEstimates);
  }

  // Merge Covariance matrix (S = sqrt(P))
  NMatrix xDiff = models[index1].stateEstimates - xMerged;
  NMatrix p1 = (models[index1].stateStandardDeviations * models[index1].stateStandardDeviations.transp() + xDiff * xDiff.transp());

  xDiff = models[index2].stateEstimates - xMerged;
  NMatrix p2 = (models[index2].stateStandardDeviations * models[index2].stateStandardDeviations.transp() + xDiff * xDiff.transp());

  NMatrix sMerged = cholesky(alpha1 * p1 + alpha2 * p2); // P merged = alpha1 * p1 + alpha2 * p2.

  // Copy merged value to first model
  models[index1].alpha = alphaMerged;
  models[index1].stateEstimates = xMerged;
  models[index1].stateStandardDeviations = sMerged;

  // Disable second model
  models[index2].isActive = false;
  models[index2].toBeActivated = false;

  return true;
}



int OppModule::getNumActiveModels()
{
  int numActive = 0;
  for (int modelID = 0; modelID < c_MAX_MODELS; modelID++){
    if(models[modelID].isActive == true) numActive++;
  }
  return numActive;
}



int OppModule::getNumFreeModels()
{
  int numFree = 0;
  for (int modelID = 0; modelID < c_MAX_MODELS; modelID++){
    if((models[modelID].isActive == false) && (models[modelID].toBeActivated == false)) numFree++;
  }
  return numFree;
}



UKF4* OppModule::getBestModel()
{
  return &(models[getBestModelID()]);
}



int OppModule::getBestModelID()
{
  // Return model with highest alpha value.
  int bestID = 0;
  for (int currID = 0; currID < c_MAX_MODELS; currID++){
    if(models[currID].isActive == false) continue; // Skip inactive models.
    if(models[currID].alpha > models[bestID].alpha) bestID = currID;
  }
  return bestID;
}






void OppModule::NormaliseAlphas()
{
  // Normalise all of the models alpha values such that all active models sum to 1.0

  // set each to one
  for (int i = 0; i < c_MAX_MODELS; i++) {
    if (models[i].isActive) {
      models[i].alpha=1.0;//models[i].alpha/sumAlpha;
    }
  }

  /*
    float sumAlpha=0.0;
    for (int i = 0; i < c_MAX_MODELS; i++) {
    if (models[i].isActive) {
    sumAlpha+=models[i].alpha;
    }
    }
    if(sumAlpha == 1) return;
    if (sumAlpha == 0) sumAlpha = 1e-12;
    for (int i = 0; i < c_MAX_MODELS; i++) {
    if (models[i].isActive) {
    models[i].alpha=models[i].alpha/sumAlpha;
    }
    }
  */
}



int OppModule::FindNextFreeModel()
{
  for (int i=0; i<c_MAX_MODELS; i++) {
    if ((models[i].isActive == true) || (models[i].toBeActivated == true)) continue;
    else return i;
  }
  return -1; // NO FREE MODELS - This is very, very bad.
}


// Reset all of the models
void OppModule::ResetAll()
{

  tlog(20, "Reset All Models");

  for(int modelNum = 0; modelNum < c_MAX_MODELS; modelNum++){
    models[modelNum].init();
  }
}



//**************************************************************************
//  This method begins the process of merging close models together

void OppModule::MergeModels(int maxAfterMerge) {
  if(maxAfterMerge <= 1) return;
  MergeModelsBelowThreshold(0.001);
  MergeModelsBelowThreshold(0.01);

  //  float threshold=0.04;
  float threshold=0.05;

  while (getNumActiveModels()>maxAfterMerge) {
    MergeModelsBelowThreshold(threshold);
    //      threshold*=5.0;
    threshold+=0.05;
  }
  return;
}



void OppModule::PrintModelStatus(int modelID)
{
  tlog(20, "Model %i, Alpha: %5.3f, Active: %i, Activate: %i", modelID,models[modelID].alpha, models[modelID].isActive, models[modelID].toBeActivated);
  return;
}



void OppModule::MergeModelsBelowThreshold(float MergeMetricThreshold)
{
  float mergeM;
  for (int i = 0; i < c_MAX_MODELS; i++) {
    for (int j = i; j < c_MAX_MODELS; j++) {
      if(i == j) continue;
      if (!models[i].isActive || !models[j].isActive ) continue;
      mergeM = fabs( MergeMetric(i,j) );
      if (mergeM < MergeMetricThreshold) { //0.5
        tlog(40, "Merging models %i, alpha: %5.3f, into model %i, alpha: %5.3f, merge metric = %5.3f, thresh = %5.3f", j, models[j].alpha, i, models[i].alpha, mergeM, MergeMetricThreshold);
        MergeTwoModels(i,j);
      }
    }
  }
}



//************************************************************************
// model to compute a metric for how 'far' apart two models are in terms of merging.
float OppModule::MergeMetric(int index1, int index2)
{
  if (index1==index2) return 10000.0;
  if (!models[index1].isActive || !models[index2].isActive ) return 10000.0; //at least one model inactive
  NMatrix xdif = models[index1].stateEstimates - models[index2].stateEstimates;
  NMatrix p1 = models[index1].stateStandardDeviations * models[index1].stateStandardDeviations.transp();
  NMatrix p2 = models[index2].stateStandardDeviations * models[index2].stateStandardDeviations.transp();

  float dij=0;
  for (int i=0; i<p1.getm(); i++) {
    dij+=(xdif[i][0]*xdif[i][0]) / (p1[i][i]+p2[i][i]);
  }
  return dij*( (models[index1].alpha*models[index2].alpha) / (models[index1].alpha+models[index2].alpha) );
}


void OppModule::mmTocm() {
  WorldObject* objs = &(worldObjects->objects_[0]);
  for (int i=0; i<NUM_WORLD_OBJS; i++) {
    objs[i].distance/=10.0;
    objs[i].visionDistance/=10.0;
    objs[i].loc.x/=10.0;
    objs[i].loc.y/=10.0;
    objs[i].endLoc.x/=10.0;
    objs[i].endLoc.y/=10.0;
    objs[i].sd.x/=10.0;
    objs[i].sd.y/=10.0;
    objs[i].absVel.x/=10.0;
    objs[i].absVel.x/=10.0;
    objs[i].relVel.y/=10.0;
    objs[i].relVel.y/=10.0;
  }
}

void OppModule::cmTomm() {
  WorldObject* objs = &(worldObjects->objects_[0]);
  for (int i=0; i<NUM_WORLD_OBJS; i++) {
    objs[i].distance*=10.0;
    objs[i].visionDistance*=10.0;
    objs[i].loc.x*=10.0;
    objs[i].loc.y*=10.0;
    objs[i].endLoc.x*=10.0;
    objs[i].endLoc.y*=10.0;
    objs[i].sd.x*=10.0;
    objs[i].sd.y*=10.0;
    objs[i].absVel.x*=10.0;
    objs[i].absVel.x*=10.0;
    objs[i].relVel.y*=10.0;
    objs[i].relVel.y*=10.0;
  }
}


void OppModule::populateOpponents() {
  int oppID = WO_OPPONENT1;

  for (int j = 0; j < c_MAX_MODELS; j++){
    if (!models[j].isActive) continue;
    if (std::isnan(models[j].stateEstimates[0][0]) || std::isnan(models[j].sd(0))) continue;

    // fill in opp with this info
    WorldObject* opp = &(worldObjects->objects_[oppID]);

    opp->loc.x = models[j].stateEstimates[0][0];
    opp->loc.y = models[j].stateEstimates[1][0];
    opp->sd.x  = models[j].sd(0);
    opp->sd.y  = models[j].sd(1);

    tlog(40, "Fill in opponent %i from model %i: opponent at (%5.3f, %5.3f), sd (%5.3f, %5.3f)", oppID, j, opp->loc.x, opp->loc.y, opp->sd.x, opp->sd.y);

    oppID++;
    if (oppID > WO_OPPONENT_LAST) break;
  }

  // for the remaining ones, put high sd
  for (; oppID <= WO_OPPONENT_LAST; oppID++){
    WorldObject* opp = &(worldObjects->objects_[oppID]);
    opp->sd.x = 10000.0;
    opp->sd.y = 10000.0;
  }

}



void OppModule::updateMemory() {
  int i=0;
  for (i=0; i < MAX_OPP_MODELS_IN_MEM; i++) {
    opponentMem->locModels[i].alpha = -1000; // set the modesl inactive by default
  }
  int tempCounter=0;
  for (i=0; i<c_MAX_MODELS; i++) {
    if (models[i].isActive) {
      OpponentModel& memModel = opponentMem->locModels[tempCounter];
      memModel.modelNumber = i;
      memModel.alpha = models[i].alpha;
      memModel.X00=models[i].stateEstimates[0][0];
      memModel.X10=models[i].stateEstimates[1][0];
      memModel.X20=models[i].stateEstimates[2][0];
      memModel.X30=models[i].stateEstimates[3][0];

      tlog(20, "Fill in opp memory %i from opp %i with sd: (%5.3f, %5.3f), last seen %i", tempCounter, i, models[i].sd(0), models[i].sd(1), models[i].frameUpdated);
      memModel.P00=models[i].sd(0); //sd(0);
      memModel.P11=models[i].sd(1); //sd(1);
      memModel.P01=fabs(models[i].stateStandardDeviations[0][1]);
      memModel.P10=memModel.P01;
      memModel.P22=models[i].sd(2); //sd(2);
      memModel.P33=models[i].sd(3);

      memModel.frameLastObserved = models[i].frameUpdated;

      // sr for sharing
      NMatrix oppSR = models[i].GetLocSR();
      memModel.SRXX = oppSR[0][0];
      memModel.SRXY = oppSR[0][1];
      memModel.SRYY = oppSR[1][1];

      tempCounter++;
    }
    if (tempCounter >= MAX_OPP_MODELS_IN_MEM) break;
  }
  opponentMem->syncModels();
}

