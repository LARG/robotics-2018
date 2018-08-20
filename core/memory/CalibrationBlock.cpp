#include <memory/CalibrationBlock.h>

CalibrationBlock::CalibrationBlock():
  gyro_x_offset_(0.0),
  gyro_y_offset_(0.0),
  gyro_z_offset_(0.0),
  gyro_x_cal_count_(0),
  gyro_y_cal_count_(0),
  gyro_z_cal_count_(0),
  slip_amount_(0.004),
  pessimistic_scale_(0.62) {}
