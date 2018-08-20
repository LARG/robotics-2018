#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include <Module.h>
#include "InertialFilter.h"

#define NUM_CALIBRATION_READINGS 50

class SensorBlock;
class FrameInfoBlock;
class WalkRequestBlock;
class SensorCalibrationBlock;
class BodyModelBlock;

class SensorModule: public Module {
 public:
  void specifyMemoryDependency();
  void specifyMemoryBlocks();
  void initSpecificModule();

  void processSensors();

 private:
  SensorBlock *raw_sensors_;
  SensorBlock *sensors_;
  FrameInfoBlock *frame_info_;
  WalkRequestBlock *walk_request_;
  SensorCalibrationBlock *sensor_calibration_;
  BodyModelBlock *body_model_;

  InertialFilter inertial_filter_;

  float stand_start_time_;
  //float calibration_rolls_[NUM_CALIBRATION_READINGS];
  //float calibration_tilts_[NUM_CALIBRATION_READINGS];
  int calibration_ind_;

  //float calibration_fsr_feet_[NUM_CALIBRATION_READINGS];
  //float calibration_fsr_left_side_[NUM_CALIBRATION_READINGS];
  //float calibration_fsr_left_front_[NUM_CALIBRATION_READINGS];
  //float calibration_fsr_right_side_[NUM_CALIBRATION_READINGS];
  //float calibration_fsr_right_front_[NUM_CALIBRATION_READINGS];

  //float calibration_inertial_[6][NUM_CALIBRATION_READINGS];

  float calibration_inertial_sum_[6];
  double last_temperature_check_time_;
  
  void filterInertial();
  void filterFSR();
  void handleCalibration();
  void setCalibration();

  float sumFSRs(int inds[], int num_fsrs);
  float getOffset(float arr[]);

  void checkTemperatures();
};

#endif /* end of include guard: SENSOR_MODULE */
