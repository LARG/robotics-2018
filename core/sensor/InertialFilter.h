#ifndef _InertialFilter_h_DEFINED
#define _InertialFilter_h_DEFINED


#include <common/NMatrix.h>

class InertialFilter {
public:
  InertialFilter();
  void init(bool in_simulation);
  void initMatrices(bool in_simulation); // now handled in the generated file initInertialFilter.cpp

  // this takes the data an perfomrs the update
  // the reason I don't pass the variables to processFrame
  // is that I want the class to extendable and we may end up using
  // information from the walk engine, and I'd prefer to not change
  // the inputs to processFrame  
  void processFrame(bool calibrated);

  // Set the intertial data from the sensors
  void setInertialData(float aX, float aY, float aZ, float gX, float gy);

  // Take in a new height for the intertial unit and recalculate C matrix
  void updateIUHeight(float torsoZ); // now handled in the generated file initInertialFilter.cpp

  // Gets for filtered and raw tilts/rolls and velocities
  float getRoll() { return filtRoll; };
  float getTilt() { return filtTilt; };
  
  float getRollVel() { return filtRollVel; };
  float getTiltVel() { return filtTiltVel; };
 
  float getRawRoll() { return rawRoll; };
  float getRawTilt() { return rawTilt; };

  void predictFuture(int numFrames, float &tilt, float &roll);

  //------ Variables ----- // Note: I generally don't believe in public/private  
private:
  // inputs and outputs
  float x_acc;
  float y_acc;
  float z_acc;
  float x_gyro;
  float y_gyro;
  
  float filtRoll;
  float filtTilt;
  float filtRollVel;
  float filtTiltVel;
  
  float rawRoll;
  float rawTilt;
  
  float gyro_scale;  // (deg/sec)
  //float iu_height; // (m)

  //float gyro_noise_rms; // (deg/sec)
  //float acc_noise_rms; // (m/s^2)
  //float motion_rms; // (rad/s^3)
  //float gyro_drift_rms; // rad/sample

  // KF variables
  NMatrix A;
  NMatrix B;
  NMatrix C;
  NMatrix Cz;
  NMatrix Cz1;
  
  NMatrix Q;  // Weighting Matricies
  NMatrix R;
  NMatrix L;
  
  NMatrix xhRoll;
  NMatrix xhTilt;
  
  NMatrix xhRollFuture;
  NMatrix xhTiltFuture;
};

#endif

