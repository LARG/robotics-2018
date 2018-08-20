#include "InertialFilter.h"

#include <math.h>
#include <iostream>
using namespace std;

#define RAD_TO_DEG  180.0/ M_PI
#define DEG_TO_RAD  M_PI/180.0

InertialFilter::InertialFilter() {
}

void InertialFilter::init(bool in_simulation) {
  x_acc = 0.0001;
  y_acc = 0.0001;
  z_acc = 0.0001;
  x_gyro = 0.0001;
  y_gyro = 0.0001;

  filtRoll=0.0;
  filtTilt=0.0;
  filtRollVel=0.0;
  filtTiltVel=0.0;
  
  rawRoll=0.0;
  rawTilt=0.0;
  
  gyro_scale = DEG_TO_RAD*0.405;  // (rad/sec) // lower is smoother, slower, higher responds faster and overshoots a lot (at only 2x)
  //numFramesToMeanOver = 30;
  
  xhRoll = NMatrix(4,1,false);
  xhTilt = NMatrix(4,1,false);
  
  xhTiltFuture = NMatrix(4,1,false);
  xhRollFuture = NMatrix(4,1,false);

  initMatrices(in_simulation);
}

void InertialFilter::processFrame(bool calibrated) {
  // First do some work on the accelerometer data
  // float abs_acc=sqrtf(x_acc*x_acc+y_acc*y_acc+z_acc*z_acc); why
  // define and cause warning? remove thisss!!!
  rawTilt=-atan2(x_acc,z_acc);
  rawRoll=-atan2(y_acc,z_acc);

  if (!calibrated) {
    xhRoll[0][0]=rawRoll; // start the filter at the position of the raw data
    xhTilt[0][0]=rawTilt; 
  }
  
  // Now work with the gyros 
  float x_g=(x_gyro) * gyro_scale;
  float y_g=(y_gyro) * gyro_scale;

  //Now do KF calculations for roll axis
  NMatrix xSen(2,1,false);
  xSen[0][0] = -y_acc; 
  xSen[1][0] = x_g;
  xhRoll=A*xhRoll-L*(C*xhRoll - xSen);
  filtRoll=convDble(Cz*xhRoll);
  filtRollVel=convDble(Cz1*xhRoll);
  
  //Now do KF calculations for tilt axis
  NMatrix ySen(2,1,false);
  ySen[0][0] = -x_acc; 
  ySen[1][0] = y_g;
  xhTilt=A*xhTilt-L*(C*xhTilt - ySen);
  filtTilt=convDble(Cz*xhTilt);
  filtTiltVel=convDble(Cz1*xhTilt);
}

void InertialFilter:: setInertialData(float aX, float aY, float aZ, float gX, float gy) {
  x_acc = aX;
  y_acc = aY;
  z_acc = aZ;
  x_gyro = gX;
  y_gyro = gy;
}

 
void InertialFilter::predictFuture(int numFrames, float &tilt, float &roll) {
  xhRollFuture = xhRoll;
  xhTiltFuture = xhTilt;
  //std::cout << xhRoll[0][0] << " " << xhRoll[1][0] << " " << xhRoll[2][0] << " " << xhRoll[3][0] << std::endl;
  for (int i = 0; i < numFrames; i++) {
    xhRollFuture = A * xhRollFuture;
    xhTiltFuture = A * xhTiltFuture;
  }
  roll = convDble(Cz*xhRollFuture);
  tilt = convDble(Cz*xhTiltFuture);
}
