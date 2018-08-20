#include "PIDController.h"
#include <math/Geometry.h>

PIDController::PIDController():
  cp_(1.0),
  ci_(0),
  cd_(0),
  eps_(0),
  current_error_(0),
  cumulative_error_(0),
  previous_error_(0)
{
}

PIDController::PIDController(float cp, float ci, float cd, float eps):
  cp_(cp),
  ci_(ci),
  cd_(cd),
  eps_(eps),
  current_error_(0),
  cumulative_error_(0),
  previous_error_(0)
{
}

float PIDController::update(float current, float target) {
  previous_error_ = current_error_;
  current_error_ = target - current;
  if (fabs(current_error_) > eps_)
    cumulative_error_ += current_error_;

  float res = cp_ * current_error_ + ci_ * cumulative_error_ + cd_ * (current_error_ - previous_error_);
  //std::cout << "curr: " << RAD_T_DEG * current << " tar: " << RAD_T_DEG * target << " res: " << RAD_T_DEG * (current + res) << std::endl;
  return res;
}

void PIDController::setParams(float cp, float ci, float cd) {
  cp_ = cp;
  ci_ = ci;
  cd_ = cd;
}

void PIDController::setParams(const Vector3<float> &params) {
  cp_ = params.x;
  ci_ = params.y;
  cd_ = params.z;
}

void PIDController::reset() {
  current_error_ = cumulative_error_ = previous_error_ = 0;
}

void PIDController::cropCumulative(float max) {
  max /= ci_;
  if (cumulative_error_ > max)
    cumulative_error_ = max;
  else if (cumulative_error_ < -max)
    cumulative_error_ = -max;
}
