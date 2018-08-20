#ifndef _UKF4_h_DEFINED
#define _UKF4_h_DEFINED

#include <math.h>
#include <common/NMatrix.h>
#include <common/WorldObject.h>

#include <memory/WorldObjectBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/FrameInfoBlock.h>

class TextLogger;


enum ukf4UpdateResult {
  UKF4_OUTLIER,
  UKF4_OK,
  UKF4_OFF  // return because this is turned off by params
};

using namespace std;

struct SmallUKF4Params {
  float kappa;
  float vel_decay_rate;
  float outlier_rejection_thresh;

  // process noise
  float robot_pos_noise;
  float robot_vel_noise;

  // std's used when resetting things
  float init_sd_x;
  float init_sd_y;
  float init_sd_vel;

};


class UKF4 {
 public:
  // Constructor
  UKF4();

  // Functions

  // Update functions
  void timeUpdate(float timePassed);
  ukf4UpdateResult opponentDetection(float distance, float bearing, float distanceErrorOffset, float distanceErrorRelative, float bearingError);
  ukf4UpdateResult linear2MeasurementUpdate(float Y1,float Y2, float SR11, float SR12, float SR22, int index1, int index2);


  // Data retrieval
  float sd(int Xi);
  float variance(int Xi);
  float getState(int stateID);
  NMatrix GetLocSR();

  // Utility
  void init();
  void reInit();
  void setMemory(WorldObjectBlock* wo, RobotStateBlock* rs, FrameInfoBlock* fi);
  void setParams(SmallUKF4Params u);
  void Reset();
  bool clipState(int stateIndex, float minValue, float maxValue);
  void setTextLogger(TextLogger* tf);
  void revert();

  // Variables

  // Multiple Models - Model state Description.
  float alpha;
  bool isActive;
  bool toBeActivated;

  // State data
  NMatrix updateUncertainties;     // Update Uncertainty.          (A matrix)
  NMatrix stateEstimates;          // State estimates.             (Xhat NMatrix)
  NMatrix stateStandardDeviations; // Standard Deviation NMatrix.   (S NMatrix)

  // Constants
  int nStates;                    // Number of states.            (Constant)
  NMatrix sqrtOfTestWeightings;    // Square root of W             (Constant)
  NMatrix sqrtOfProcessNoise;      // Square root of Process Noise (Q matrix). (Constant)
  NMatrix sqrtOfProcessNoiseReset; // Square root of Q for reset.  (Conastant)

  // Tuning Values 
  SmallUKF4Params ukfParams;
  
  TextLogger* textlogger;

  float lastInnov2;
  NMatrix lastStateEstimates;
  NMatrix lastStateStandardDeviations;
  int lastFrameUpdated;

  int frameUpdated;

  // memory
  WorldObjectBlock* worldObjects;
  RobotStateBlock* robotState;
  FrameInfoBlock* frameInfo;
};

#endif
