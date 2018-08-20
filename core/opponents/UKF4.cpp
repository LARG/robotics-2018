/**
   4 states calculation
   Xhat[0][0]=oppx
   Xhat[1][0]=oppy
   Xhat[2][0]=opp vel x
   Xhat[3][0]=opp vel y
**/

#include "UKF4.h"
#include <iostream>
#include <opponents/Logging.h>

UKF4::UKF4()
{
  /**************************Initialization**************************/
  /*This is where values can be adjusted*/

  alpha = 1.0; // Accuracy of model (0.0 -> 1.0)
  isActive = false; // Model currently in use.
  toBeActivated = false; // Model to be in use.

  lastInnov2 = 999;
  frameUpdated = -1000;

  // Set Update Uncertainty
  updateUncertainties = NMatrix(4, 4, true);
  updateUncertainties[2][2] = 0.975; // velocity x
  updateUncertainties[3][3] = 0.975; // velocity y
  
  //Initialisation of Xhat and S
  init();

  // Process Noise - NMatrix Square Root of Q
  // For regular update
  // Todd: I doubled these
  sqrtOfProcessNoise = NMatrix(4,4,true);
  sqrtOfProcessNoise[0][0] = 2.0* 2.0; // Robot X coord.
  sqrtOfProcessNoise[1][1] = 2.0* 2.0; // Robot Y coord.
  sqrtOfProcessNoise[2][2] = 2.0* 3.16228; // Robot X Velocity.
  sqrtOfProcessNoise[3][3] = 2.0* 3.16228; // Robot Y Velocity.

  // side of the field if it managed to localise to the incorrect half.
  sqrtOfProcessNoiseReset = NMatrix(4,4,false);
  sqrtOfProcessNoiseReset[0][0] = 150.0; // extra 150cm sd 
  sqrtOfProcessNoiseReset[1][1] = 100.0; // extra 100cm sd

  nStates = stateEstimates.getm(); // number of states.

  // Create square root of W matrix
  // This is used to weight the sigma points.
  sqrtOfTestWeightings = NMatrix(1,2*nStates+1,false);
  sqrtOfTestWeightings[0][0] = sqrtf(1.0/(nStates+1.0));
  float outerWeighting = sqrtf(1.0/(2*(nStates+1.0)));
  for(int i=1; i <= 2*nStates; i++){
    sqrtOfTestWeightings[0][i] = (outerWeighting);
  }
  return;
}

void UKF4::setTextLogger(TextLogger* tf){
  textlogger = tf;
}

void UKF4::setMemory(WorldObjectBlock* wo, RobotStateBlock* rs, FrameInfoBlock* fi) {
  worldObjects = wo;
  robotState = rs;
  frameInfo = fi;
}

void UKF4::init()
{
  alpha = 1.0;
  // Initial state estimates
  stateEstimates= NMatrix(4,1,true);

  // S = Standard deviation matrix.
  // Initial Uncertainty
  stateStandardDeviations = NMatrix(4,4,false);
  stateStandardDeviations[0][0] = 150; // 150 cm
  stateStandardDeviations[1][1] = 100; // 100 cm
  stateStandardDeviations[2][2] = 10;  // 10 cm/s
  stateStandardDeviations[3][3] = 10;  // 10 cm/s
}

void UKF4::reInit(){
  for (int i = 0; i < 4; i++){
    stateEstimates[i][0]  = 0.0;
  } 
  alpha = 1.0;
  lastInnov2 = 999;

  stateStandardDeviations[0][0] = ukfParams.init_sd_x;
  stateStandardDeviations[1][1] = ukfParams.init_sd_y;
  stateStandardDeviations[2][2] = ukfParams.init_sd_vel;
  stateStandardDeviations[3][3] = ukfParams.init_sd_vel;
}

void UKF4::setParams(SmallUKF4Params u){
  ukfParams = u;

  // set stuff that uses these params
  updateUncertainties[2][2] = ukfParams.vel_decay_rate; // vel x
  updateUncertainties[3][3] = ukfParams.vel_decay_rate; // velocity y

  stateStandardDeviations[0][0] = ukfParams.init_sd_x;
  stateStandardDeviations[1][1] = ukfParams.init_sd_y;
  stateStandardDeviations[2][2] = ukfParams.init_sd_vel;
  stateStandardDeviations[3][3] = ukfParams.init_sd_vel;

  sqrtOfProcessNoise[0][0] = ukfParams.robot_pos_noise;    // Robot X coord.
  sqrtOfProcessNoise[1][1] = ukfParams.robot_pos_noise;    // Robot Y coord.
  sqrtOfProcessNoise[2][2] = ukfParams.robot_vel_noise;     // Robot X Velocity.
  sqrtOfProcessNoise[3][3] = ukfParams.robot_vel_noise;     // Robot Y Velocity.

  sqrtOfProcessNoiseReset[0][0] = ukfParams.init_sd_x;     // extra 150cm sd
  sqrtOfProcessNoiseReset[1][1] = ukfParams.init_sd_y;     // extra 100cm sd

}


void UKF4::timeUpdate(float timePassed)
{

  updateUncertainties[0][2] = timePassed; // x velx
  updateUncertainties[1][3] = timePassed; // y vely



  //-----------------------Update for velocity
  // Estimated velocity and position, combined with an estimated decay on velocity (friction) is used to predict the new position
  stateEstimates[0][0] += stateEstimates[2][0]*timePassed; // Update x position by x velocity.
  stateEstimates[1][0] += stateEstimates[3][0]*timePassed; // Update y position by y velocity.
  stateEstimates[2][0] *= ukfParams.vel_decay_rate; // Reduce x velocity assuming deceleration
  stateEstimates[3][0] *= ukfParams.vel_decay_rate; // Reduce y velocity assuming deceleration

  // Householder transform. Unscented KF algorithm. Takes a while.
  stateStandardDeviations=HT(horzcat(updateUncertainties*stateStandardDeviations, sqrtOfProcessNoise));

  return;
}


void UKF4::revert()
{
  stateStandardDeviations = lastStateStandardDeviations;
  stateEstimates = lastStateEstimates;
  frameUpdated = lastFrameUpdated;
}


// RHM 7/7/08: Additional function for resetting
void UKF4::Reset()
{
  // Add extra uncertainty
  stateStandardDeviations = HT(horzcat(stateStandardDeviations, sqrtOfProcessNoiseReset));
}


// update based on an opponent detection
ukf4UpdateResult UKF4::opponentDetection(float obsDistance, float obsBearing, float distanceErrorOffset, float distanceErrorRelative, float bearingError)
{

  WorldObject* self = &(worldObjects->objects_[robotState->WO_SELF]);

  tlog(70, "update using self %i, loc (%5.3f, %5.3f), orient %5.3f, sd (%5.5f, %5.5f), sd orient %5.5f", robotState->WO_SELF, self->loc.x, self->loc.y, self->orientation, self->sd.x, self->sd.y, self->sdOrientation);

  // TODO: incorporate robot's own sd in x,y,theta
  float oppX = obsDistance * cos(obsBearing + self->orientation) + self->loc.x;
  float oppY = obsDistance * sin(obsBearing + self->orientation) + self->loc.y;

  tlog(70, "update opp filter with robot at dist %5.3f, bear %5.3f, global (%5.3f, %5.3f)", obsDistance, obsBearing, oppX, oppY);

  // if robot is off field.... return outlier
  if (fabs(oppX) > (GRASS_X/20.0) || fabs(oppY) > (GRASS_Y/20.0)){
    tlog(10, "observed opponent off field, outlier");
    return UKF4_OUTLIER;
  }

  float R_range = distanceErrorOffset + distanceErrorRelative * powf(obsDistance, 2);
  float R_bearing = bearingError;

  // Calculate update uncertainties - S_obj_rel & R_obj_rel
  NMatrix S_obj_rel = NMatrix(2,5,false);

  // Todd: convert dist/bearing/locx/y/theta error to abs x/y error
  S_obj_rel[0][0] = cos(obsBearing + self->orientation) * sqrtf(R_range);              // oppX/drange   * r_range
  S_obj_rel[0][1] = -sin(obsBearing + self->orientation)*obsDistance*sqrtf(R_bearing);   // oppX/dbear    * r_bear
  S_obj_rel[0][2] = self->sd.x;             // oppX/locx     * r_locx
  S_obj_rel[0][3] = 0.0;             // oppX/locy     * r_locy
  S_obj_rel[0][4] = -sin(obsBearing+self->orientation)*obsDistance*self->sdOrientation;             // oppX/theta    * r_theta

  S_obj_rel[1][0] = sin(obsBearing + self->orientation) * sqrtf(R_range);  // oppY/drange   * r_range
  S_obj_rel[1][1] = cos(obsBearing + self->orientation) * obsDistance * sqrtf(R_bearing);             // oppY/dbear    * r_bear
  S_obj_rel[1][2] = 0.0;             // oppY/locx     * r_locx
  S_obj_rel[1][3] = self->sd.y;             // oppY/locy     * r_locy
  S_obj_rel[1][4] = cos(obsBearing + self->orientation) * obsDistance * self->sdOrientation;             // oppY/theta    * r_theta

  //  tlog(10, "sd range %5.5f, sd bearing %5.5f, sdx %5.5f, sdy %5.5f, sdtheta %5.5f", R_range, R_bearing, self->sd.x, self->sd.y, self->sdOrientation);
  //tlog(10, "from those errors to globalx: %5.3f, %5.3f, %5.3f, %5.3f, %5.3f", S_obj_rel[0][0], S_obj_rel[0][1], S_obj_rel[0][2], S_obj_rel[0][3], S_obj_rel[0][4]);
  //tlog(10, "from those errors to globaly: %5.3f, %5.3f, %5.3f, %5.3f, %5.3f", S_obj_rel[1][0], S_obj_rel[1][1], S_obj_rel[1][2], S_obj_rel[1][3], S_obj_rel[1][4]);

  NMatrix R_obj_rel = S_obj_rel * S_obj_rel.transp(); // R = S^2

  // Unscented KF Stuff.
  NMatrix yBar;                                    //reset
  NMatrix Py;
  NMatrix Pxy=NMatrix(4, 2, false);                    //Pxy=[0;0;0];
  NMatrix scriptX=NMatrix(stateEstimates.getm(), 2 * nStates + 1, false);
  scriptX.setCol(0, stateEstimates);                         //scriptX(:,1)=Xhat;

  //----------------Saturate ScriptX angle sigma points to not wrap
  for(int i=1;i<nStates+1;i++){//hack to make test points distributed
    // Addition Portion.
    scriptX.setCol(i, stateEstimates + sqrtf((float)nStates + ukfParams.kappa) * stateStandardDeviations.getCol(i - 1));
    // Subtraction Portion.
    scriptX.setCol(nStates + i,stateEstimates - sqrtf((float)nStates + ukfParams.kappa) * stateStandardDeviations.getCol(i - 1));
  }
  //----------------------------------------------------------------
  NMatrix scriptY = NMatrix(2, 2 * nStates + 1, false);
  NMatrix temp = NMatrix(2, 1, false);

  for(int i = 0; i < 2 * nStates + 1; i++){
    // expected values for each sigma point (abs x/y location)
    temp[0][0] = scriptX[0][i];
    temp[1][0] = scriptX[1][i];
    scriptY.setCol(i, temp.getCol(0));
  }
  NMatrix Mx = NMatrix(scriptX.getm(), 2 * nStates + 1, false);
  NMatrix My = NMatrix(scriptY.getm(), 2 * nStates + 1, false);
  for(int i = 0; i < 2 * nStates + 1; i++){
    Mx.setCol(i, sqrtOfTestWeightings[0][i] * scriptX.getCol(i));
    My.setCol(i, sqrtOfTestWeightings[0][i] * scriptY.getCol(i));
  }

  NMatrix M1 = sqrtOfTestWeightings;
  yBar = My * M1.transp(); // Predicted Measurement.
  Py = (My - yBar * M1) * (My - yBar * M1).transp();
  Pxy = (Mx - stateEstimates * M1) * (My - yBar * M1).transp();

  NMatrix K = Pxy * Invert22(Py + R_obj_rel); // K = Kalman filter gain.

  NMatrix y = NMatrix(2,1,false); // Measurement. I terms of relative (x,y).
  y[0][0] = oppX;
  y[1][0] = oppY; 
  //end of standard ukf stuff
  //RHM: 20/06/08 Outlier rejection.
  float innovation2 = convDble((yBar - y).transp() * Invert22(Py + R_obj_rel) * (yBar - y));

  // Update Alpha
  float innovation2measError = convDble((yBar - y).transp() * Invert22(R_obj_rel) * (yBar - y));
  alpha *= 1 / (1 + innovation2measError);

  tlog(70, "Y: %5.3f, %5.3f, Ybar: %5.3f, %5.3f, yBar-y: %5.3f, %5.3f", y[0][0], y[1][0], yBar[0][0], yBar[1][0], yBar[0][0]-y[0][0], yBar[1][0]-y[1][0]);

  tlog(70, "innovation2: %5.3f, outlier thresh: %5.3f", innovation2, ukfParams.outlier_rejection_thresh);

  lastInnov2 = innovation2;
  lastStateEstimates = stateEstimates;
  lastStateStandardDeviations = stateStandardDeviations;
  lastFrameUpdated = frameUpdated;


  if (innovation2 > ukfParams.outlier_rejection_thresh){
    return UKF4_OUTLIER;
  }
  if (innovation2 < 0){
    //std::cout<<"**********************************"<<std::endl;
    //Py.print();
    //R_obj_rel.print();
    //std::cout<<"**********************************"<<std::endl;
  }


  // only update this from actual seen observations
  frameUpdated = frameInfo->frame_id;


  tlog(70, "Y: %5.3f, %5.3f, Ybar: %5.3f, %5.3f, yBar-y: %5.3f, %5.3f", y[0][0], y[1][0], yBar[0][0], yBar[1][0], yBar[0][0]-y[0][0], yBar[1][0]-y[1][0]);

  NMatrix change = NMatrix(4,1,false) - K*(yBar - y);
  //  tlog(70, "Change: %5.3f, %5.3f, %5.3f, %5.3f", change[0][0], change[1][0], change[2][0], change[3][0]);

  stateStandardDeviations = HT( horzcat(Mx - stateEstimates*M1 - K*My + K*yBar*M1, K*S_obj_rel) );
  stateEstimates = stateEstimates - K*(yBar - y);
  return UKF4_OK;
}





float UKF4::variance(int Xi){
  return convDble(stateStandardDeviations.getRow(Xi)*stateStandardDeviations.getRow(Xi).transp());
}



float UKF4::sd(int Xi){
  return sqrtf(variance(Xi));
}



float UKF4::getState(int stateID){
  return stateEstimates[stateID][0];
}




bool UKF4::clipState(int stateIndex, float minValue, float maxValue){
  bool clipped = false;
  if(stateEstimates[stateIndex][0] > maxValue){
    float mult, Pii;
    NMatrix Si;
    Si = stateStandardDeviations.getRow(stateIndex);
    Pii = convDble(Si * Si.transp());
    mult = (stateEstimates[stateIndex][0] - maxValue) / Pii;
    stateEstimates = stateEstimates - mult * stateStandardDeviations * Si.transp();
    stateEstimates[stateIndex][0] = maxValue;
    clipped = true;
  }
  if(stateEstimates[stateIndex][0] < minValue){
    float mult, Pii;
    NMatrix Si;
    Si = stateStandardDeviations.getRow(stateIndex);
    Pii = convDble(Si * Si.transp());
    mult = (stateEstimates[stateIndex][0] - minValue) / Pii;
    stateEstimates = stateEstimates - mult * stateStandardDeviations * Si.transp();
    stateEstimates[stateIndex][0] = minValue;
    clipped = true;
  }
  return clipped;
}

//
//RHM: 26/6/08 New function, needed for shared ball stuff (and maybe others....)
//======================================================================
//
ukf4UpdateResult UKF4::linear2MeasurementUpdate(float Y1,float Y2, float SR11, float SR12, float SR22, int index1, int index2)
//
// KF update (called for example by shared ball) where a pair of measurements, [Y1;Y2];
// with variance Square Root of Covariance (SR) [SR11 SR12; 0 SR22]
// are predicted to be Xhat[index1][0] and Xhat[index2][0] respectively
// This is based on the function fieldObjectmeas, but simplified.
//
// Example Call (given data from wireless: ballX, ballY, SRballXX, SRballXY, SRballYY)
//      linear2MeasurementUpdate( ballX, ballY, SRballXX, SRballXY, SRballYY, 3, 4 )
{
  NMatrix SR = NMatrix(2,2,false);
  SR[0][0] = SR11;
  SR[0][1] = SR12;
  SR[1][1] = SR22;

  NMatrix R = NMatrix(2,2,false);
  R = SR * SR.transp();

  NMatrix Py = NMatrix(2, 2, false);
  NMatrix Pxy = NMatrix(4, 2, false);

  NMatrix CS = NMatrix(2, 4, false);
  CS.setRow(0, stateStandardDeviations.getRow(index1));
  CS.setRow(1, stateStandardDeviations.getRow(index2));

  Py = CS * CS.transp();
  Pxy = stateStandardDeviations * CS.transp();

  NMatrix K = Pxy * Invert22(Py + R);   //Invert22

  NMatrix y = NMatrix(2, 1, false);
  y[0][0] = Y1;
  y[1][0] = Y2;

  NMatrix yBar = NMatrix(2, 1, false); //Estimated values of the measurements Y1,Y2
  yBar[0][0] = stateEstimates[index1][0];
  yBar[1][0] = stateEstimates[index2][0];
  //RHM: (3) Outlier rejection.
  float innovation2 = convDble( (yBar - y).transp() * Invert22(Py + SR * SR.transp()) * (yBar - y) );

  //  tlog(30, "innovation2: %5.3f, outlier thresh: %5.3f", innovation2, ukfParams.outlier_rejection_thresh);

  lastInnov2 = innovation2;
  lastStateEstimates = stateEstimates;
  lastStateStandardDeviations = stateStandardDeviations;
  lastFrameUpdated = frameUpdated;

  if (innovation2 > ukfParams.outlier_rejection_thresh){
    //std::cout<<"+++++++++++++++++not update+++++++++++++++++"<<std::endl;
    return UKF4_OUTLIER;
  }
  if(innovation2 < 0){
    //std::cout<<"**********************************"<<std::endl;
    //Py.print();
    //std::cout<<"**********************************"<<std::endl;
  }

  stateStandardDeviations = HT(horzcat(stateStandardDeviations - K * CS, K * SR));
  stateEstimates = stateEstimates - K * (yBar - y);
  return UKF4_OK;
}

NMatrix UKF4::GetLocSR(){
  return HT(vertcat(stateStandardDeviations.getRow(0), stateStandardDeviations.getRow(1)));
}
