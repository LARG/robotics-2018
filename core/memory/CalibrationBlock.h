#pragma once

#include <memory/MemoryBlock.h>
#include <schema/gen/CalibrationBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(class CalibrationBlock : public MemoryBlock {
public:
  SCHEMA_METHODS(CalibrationBlock);
  CalibrationBlock();
  SCHEMA_FIELD(double gyro_x_offset_);
  SCHEMA_FIELD(double gyro_y_offset_);
  SCHEMA_FIELD(double gyro_z_offset_ = 0);
  SCHEMA_FIELD(int gyro_x_cal_count_);
  SCHEMA_FIELD(int gyro_y_cal_count_);
  SCHEMA_FIELD(int gyro_z_cal_count_);
  SCHEMA_FIELD(double slip_amount_ = 0);
  SCHEMA_FIELD(double pessimistic_scale_ = 1);
  SCHEMA_FIELD(float slip_average_ = 0);
  SCHEMA_FIELD(float last_gyro_z_ = 0);
  SCHEMA_FIELD(float last_gyro_theta_ = 0);
  SCHEMA_FIELD(float avg_gyro_z_ = 0);
  SCHEMA_FIELD(float avg_delta_gyro_z_ = 10);
  SCHEMA_FIELD(float calibration_write_time_ = -1);
  SCHEMA_FIELD(float last_calibration_write_ = -1);
  SCHEMA_FIELD(float last_gyro_z_time_);
});
