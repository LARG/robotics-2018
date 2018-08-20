#ifndef FORWARDKINEMATICS_GJA13VPQ
#define FORWARDKINEMATICS_GJA13VPQ

#include <math/Pose3D.h>
#include <math/Vector3.h>
#include <common/TiltRoll.h>
#include <common/MassCalibration.h>
#include <common/RobotDimensions.h>

namespace ForwardKinematics {
  using namespace std;
  void calculateRelativePose(vector<float> joint_angles, Pose3D *rel_parts, vector<float> dimensions);
  void calculateRelativePose(float *joint_angles, Pose3D *rel_parts, float* dimensions);
  void calculateRelativePose(vector<float> joint_angles, float angleX, float angleY, Pose3D *rel_parts, vector<float> dimensions);
  void calculateRelativePose(float *joint_angles, float angleX, float angleY, Pose3D *rel_parts, float* dimensions);
  void calculateAbsolutePose(vector<float> sensors, Pose3D *rel_parts, Pose3D *abs_parts);
  void calculateAbsolutePose(float* sensors, Pose3D *rel_parts, Pose3D *abs_parts);
  void calculateAbsolutePose(Pose3D base, Pose3D *rel_parts, Pose3D *abs_parts);
  void calculateCoM(Pose3D *abs_parts, Vector3<float> &center_of_mass, const MassCalibration &mass_calibration); /**< Calculates the position of the center of mass relative to the robot's origin. */
  Pose3D calculateVirtualBase(bool useLeft, Pose3D *rel_parts);
  Pose3D calculateVirtualBase(vector<float> sensors, Pose3D *rel_parts);
  Pose3D calculateVirtualBase(float *sensors, Pose3D *rel_parts);
  TiltRoll calculateTiltRollFromLeg(bool left, float *joint_angles, const RobotDimensions &dimensions);
}

#endif /* end of include guard: FORWARDKINEMATICS_GJA13VPQ */
