/**
* @file TorsoMatrix.h
* Declaration of class TorsoMatrix.
* @author Colin Graf
*/ 

#ifndef TorsoMatrix_H
#define TorsoMatrix_H

#include <math/Pose3D.h>

/**
* @class TorsoMatrix
* Matrix describing the transformation from ground to the robot torso.
*/
class TorsoMatrix : public Pose3D
{
public:  
  Pose3D offset; /**< The estimated offset (including odometry) from last torso matrix to this one. (relative to the torso) */
  bool isValid; /**< Matrix is only valid if robot is on ground. */

  /** Default constructor. */
  TorsoMatrix() : isValid(false) {}
  
};

class TorsoMatrixPrev : public TorsoMatrix {};

#endif //TorsoMatrix_H
