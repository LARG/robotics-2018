#ifndef OPP_MODULE_H
#define OPP_MODULE_H

#include "UKF4.h"
#include <Module.h>

#include <memory/MemoryFrame.h>

#include <memory/OpponentBlock.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/GameStateBlock.h>
#include <memory/ProcessedSonarBlock.h>

#include <math/Geometry.h>
#include <common/WorldObject.h>
#include <common/Field.h>

// params for the ukf
struct OppLocParams{

  // R values for measurements of different types
  float R_vision_theta;
  float R_vision_range_offset;
  float R_vision_range_relative;
  float ignore_vision_dist;

  float R_sonar_theta;
  float R_sonar_range_offset;
  float R_sonar_range_relative;
  float ignore_sonar_dist;

  float R_bump_theta;
  float R_bump_range;
  float bump_distance_estimate;

  float kappa;
  float vel_decay_rate;
  float outlier_rejection_thresh;

  // process noise
  float robot_pos_noise;
  float robot_vel_noise;
  float team_sd_factor;

  // std's used when resetting things
  float init_sd_x;
  float init_sd_y;
  float init_sd_vel;

  // num models
  int max_models_after_merge;

  bool USE_VISION;
  bool USE_SONAR;
  bool USE_BUMP;
  bool USE_TEAM;

};


class OppModule: public Module
{
 public:
  void specifyMemoryDependency();
  void specifyMemoryBlocks();
  void initSpecificModule();
  void reInit();

  void processFrame();

  void ResetAll();
  void runTimeUpdates();

  bool clipModelToField(int modelID);
  bool clipActiveModelsToField();
  
  void doPenaltyKickReset();
  void doOpponentReset();

  void checkOpponentTeamPackets();
  int doOpponentVisionUpdate();
  int doOpponentSonarUpdate();
  int doOpponentBumpUpdate(float heading);
  int doSharedOpponentUpdate(float x, float y, float sdxx, float sdxy, float sdyy);

  int getNumActiveModels();
  int getNumFreeModels();
  bool initNewModel(int modelID, float oppDist, float oppAng);

  UKF4* getBestModel();
  int getBestModelID();
  void NormaliseAlphas();
  int FindNextFreeModel();
  bool MergeTwoModels(int index1, int index2);
  float MergeMetric(int index1, int index2);
  void MergeModels(int maxAfterMerge);
  void MergeModelsBelowThreshold(float MergeMetricThreshold);
  void PrintModelStatus(int modelID);

 
  // fill world obj with loc info
  void populateOpponents();

  // Model Reset Functions
  void resetSdMatrix(int modelNumber);

  // helper methods for python
  bool isModelActive(int index) {return models[index].isActive;};
  UKF4 *getModelPtr(int index) {return &(models[index]);};
  void updateMemory();

  // convert between mm and cm
  void mmTocm();
  void cmTomm();

  // Multiple Models Stuff
  static const int c_MAX_MODELS = 34; //(c_MAX_MODELS_AFTER_MERGE*8+2); // Total models
  UKF4 tempModel;
  UKF4 models[c_MAX_MODELS];

  OppLocParams ukfParams;

  // timing info
  double timeLast;
  double timePassed;

  // previous state info
  int previousGameState;
  int previousTeam;

  // memory info
  WorldObjectBlock* worldObjects;
  OpponentBlock* opponentMem;
  TeamPacketsBlock* teamPacketsMem;
  FrameInfoBlock* frameInfo;
  RobotStateBlock* robotState;
  GameStateBlock* gameState;
  ProcessedSonarBlock* processedSonar;

};

#endif
