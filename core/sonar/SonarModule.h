#ifndef SONARMODULE_RGN30PU
#define SONARMODULE_RGN30PU

#include <Module.h>
#include <common/RingBufferWithSum.h>

class FrameInfoBlock;
class JointCommandBlock;
class SensorBlock;
class ProcessedSonarBlock;
class RobotStateBlock;

namespace Sonar {

  const float MIN_DISTANCE = 0.1; //was 0.2, but it can sense 0.1
  const float MAX_DISTANCE_V4 = 0.35; // 0.5
  const float MAX_DISTANCE_V5 = 0.35; // 0.9
  const float LR_DISTANCE_DIFF_V4 = 0.2;
  const float LR_DISTANCE_DIFF_V5 = 0.2;
  const float CONFIDENCE = 0.6;
  const float CONF_HISTORY_WEIGHT = 0.5;
  const float DIST_HISTORY_WEIGHT = 0.5;

  const int SAME_READING_THRESHOLD = 75;

  enum Command {
    LEFT_TO_LEFT = 0,
    LEFT_TO_RIGHT = 1,
    RIGHT_TO_LEFT = 2,
    RIGHT_TO_RIGHT = 3,
    RECEIVE_BOTH = 4,
    TRANSMIT_BOTH = 8,
    RESET = 32,
    PERIODICALLY = 64
  };

};

class SonarModule: public Module {
 public:
  void specifyMemoryDependency();
  void specifyMemoryBlocks();
  void initSpecificModule();

  void processFrame();

 private:

  void addDistance(float distance, int index);
  void filterSonarsFromConfidence();
  void filterSonarsFromMedian();

  void initSonar();
  void processSonars();
  void processCommands();

  FrameInfoBlock *frame_info_;
  JointCommandBlock *commands_;
  SensorBlock *sensors_;
  ProcessedSonarBlock *processed_sonar_;
  RobotStateBlock* robot_state_;
  
  std::vector<float> modes_;
  unsigned int mode_ind_;
  // intervals are in seconds
  float read_interval_;
  float send_interval_;
  float switch_interval_;
  float ignore_after_switch_interval_;
  // when commands were last sent
  float last_read_time_;
  float last_send_time_;
  float last_switch_time_;
  float min_distance_;
  float max_distance_;
  float lr_distance_diff_;

  // Distance and confidence for left and right side
  float confidence_[2];
  float distance_[2];

  // Ring buffer for median
  RingBufferWithSum<float, 5> left_buf_;
  RingBufferWithSum<float, 5> right_buf_;

  // Some variables to determine if the sonar has stalled
  int sameReadingCount;
  float prevLeftReading;
  float prevRightReading;
  bool sonarModuleEnabled;

  std::map<float, std::string> command_map_;

};

#endif /* end of include guard: SONARMODULE_RGN30PU */
