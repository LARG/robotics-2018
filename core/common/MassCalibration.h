/**
* @file MassCalibration.h
* Declaration of a class for representing the relative positions and masses of mass points.
* @author <a href="mailto:allli@informatik.uni-bremen.de">Alexander Härtl</a>
*/

#ifndef __MassCalibration_H__
#define __MassCalibration_H__

#include "math/Vector3.h"
#include "RobotInfo.h"

class MassCalibration
{
public:

  /** 
  * Information on the mass distribution of a limb of the robot.
  */
  class MassInfo
  {
  public:
    float mass; /**< The mass of this limb. */
    Vector3<float> offset; /**< The offset of the center of mass of this limb relative to its hinge. */

    /**
    * Default constructor.
    */
    MassInfo() : mass(0), offset() {}

		void set(float m, float x, float y, float z)
		{
			mass = m;
			offset = Vector3<float>(x,y,z);
		}
  };

  MassInfo masses[BodyPart::NUM_PARTS]; /**< Information on the mass distribution of all joints. */
  

  // Nao 3.3 (old lower arms 
  MassCalibration()
	{
    setNaoH21();
  }
  
  void setNaoH21() {
    // v4 with normal hands
		masses[BodyPart::neck].set(64.42,-0.01,0.14,-27.42);
		masses[BodyPart::head].set(605.33,-1.12,0.03,52.58);
    masses[BodyPart::top_camera].set(0,0,0,0);
    masses[BodyPart::bottom_camera].set(0,0,0,0);
		
    masses[BodyPart::left_shoulder].set(75.04,-1.65,-26.63,0.14);
		masses[BodyPart::left_bicep].set(157.77,24.55,5.63,3.30);
		masses[BodyPart::left_elbow].set(64.83,-27.44,0.0,-0.14);
		masses[BodyPart::left_forearm].set(184.05,65.30,1.14,0.51);
		masses[BodyPart::left_hand].set(0,0,0,0);

		masses[BodyPart::right_shoulder].set(75.04,-1.65,-26.63,0.14);
		masses[BodyPart::right_bicep].set(157.94,24.29,-9.52,3.20);
		masses[BodyPart::right_elbow].set(64.83,-27.44,0.0,-0.14);
		masses[BodyPart::right_forearm].set(184.05,65.30,1.14,0.51);
		masses[BodyPart::right_hand].set(0,0,0,0);

    masses[BodyPart::left_pelvis].set(69.81,-7.81,-11.14,26.61);
		masses[BodyPart::left_hip].set(130.53,-15.49,0.29,-5.15);
		masses[BodyPart::left_thigh].set(389.68,1.38,2.21,-53.73);
		masses[BodyPart::left_tibia].set(291.42,4.53,2.25,-49.36);
		masses[BodyPart::left_ankle].set(134.15,0.45,0.29,6.85);
		masses[BodyPart::left_foot].set(161.71,25.40,3.30,-32.39);
		masses[BodyPart::left_bottom_foot].set(0,0,0,0);

		masses[BodyPart::right_pelvis].set(71.18,-7.66,12.00,27.16);
		masses[BodyPart::right_hip].set(130.53,-15.49,-0.29,-5.16);
		masses[BodyPart::right_thigh].set(389.76,1.39,-2.25,-53.74);
		masses[BodyPart::right_tibia].set(291.63,3.94,-2.21,-49.38);
		masses[BodyPart::right_ankle].set(134.15,0.45,-0.30,6.84);
		masses[BodyPart::right_foot].set(161.71,25.40,-3.32,-32.39);
		masses[BodyPart::right_bottom_foot].set(0,0,0,0);

		masses[BodyPart::torso].set(1049.56,-4.13,0.09,128.42); //z=85+43.42
  }

  void setNaoH24() {
    // v4 with motorized hands
    setNaoH21();
		masses[BodyPart::left_forearm].set( 185.33 + 77.61,65.30,1.14,0.51); // only updated the weights
		masses[BodyPart::right_forearm].set(185.33 + 77.78,65.30,1.14,0.51); // only updated the weights
  }

  void setNao33() {
		masses[BodyPart::neck].set(59.39,-0.02,0.17,-25.56);
		masses[BodyPart::head].set(520.65,1.20,-0.84,53.53);
    masses[BodyPart::top_camera].set(0,0,0,0);
    masses[BodyPart::bottom_camera].set(0,0,0,0);
		masses[BodyPart::left_shoulder].set(69.96,-1.78,24.96,0.18);
		masses[BodyPart::left_bicep].set(123.09,18.85,-5.77,0.65);
		masses[BodyPart::left_elbow].set(59.71,-25.60,0.01,-0.19);
		masses[BodyPart::left_forearm].set(112.82,69.92,-0.96,-1.14);
		masses[BodyPart::left_hand].set(0,0,0,0);
		masses[BodyPart::right_shoulder].set(69.96,-1.78,24.96,0.18);
		masses[BodyPart::right_bicep].set(123.09,18.85,-5.77,0.65);
		masses[BodyPart::right_elbow].set(59.71,-25.60,0.01,-0.19);
		masses[BodyPart::right_forearm].set(112.82,69.92,-0.96,-1.14);
		masses[BodyPart::right_hand].set(0,0,0,0);
		masses[BodyPart::left_pelvis].set(71.77,-7.66,12.00,27.17);
		masses[BodyPart::left_hip].set(135.3,-16.49,-0.29,-4.75);
		masses[BodyPart::left_thigh].set(394.21,1.32,-2.35,-53.52);
		masses[BodyPart::left_tibia].set(291.59,4.22,-2.52,-48.68);
		masses[BodyPart::left_ankle].set(138.92,1.42,-0.28,6.38);
		masses[BodyPart::left_foot].set(161.75,25.40,-3.32,-32.41);
		masses[BodyPart::left_bottom_foot].set(0,0,0,0);
		masses[BodyPart::right_pelvis].set(71.77,-7.66,12.00,27.17);
		masses[BodyPart::right_hip].set(135.3,-16.49,-0.29,-4.75);
		masses[BodyPart::right_thigh].set(394.21,1.32,-2.35,-53.52);
		masses[BodyPart::right_tibia].set(291.59,4.22,-2.52,-48.68);
		masses[BodyPart::right_ankle].set(138.92,1.42,-0.28,6.38);
		masses[BodyPart::right_foot].set(161.75,25.40,-3.32,-32.41);
		masses[BodyPart::right_bottom_foot].set(0,0,0,0);
		masses[BodyPart::torso].set(1039.48,-4.15,0.07,127.58); //z=85+42.27
  }
  
  
  // Nao 3.2 
	void setNao32() {
		masses[BodyPart::neck].set(59.59,-0.03,0.18,-25.73);
		masses[BodyPart::head].set(476.71,3.83,-0.93,51.56);
    masses[BodyPart::top_camera].set(0,0,0,0);
    masses[BodyPart::bottom_camera].set(0,0,0,0);
		masses[BodyPart::left_shoulder].set(69.84,-1.78,-25.07,0.19);
		masses[BodyPart::left_bicep].set(121.66,20.67,3.88,3.62);
		masses[BodyPart::left_elbow].set(59.59,-25.73,-0.01,-0.2);
		masses[BodyPart::left_forearm].set(112.82,69.92,0.96,-1.14);
		masses[BodyPart::left_hand].set(0,0,0,0);
		masses[BodyPart::right_shoulder].set(69.84,-1.78,25.07,0.19);
		masses[BodyPart::right_bicep].set(121.66,20.67,-3.88,3.62);
		masses[BodyPart::right_elbow].set(59.59,-25.73,0.01,-0.2);
		masses[BodyPart::right_forearm].set(112.82,69.92,-0.96,-1.14);
		masses[BodyPart::right_hand].set(0,0,0,0);
		masses[BodyPart::left_pelvis].set(72.44,-7.17,-11.87,27.05);
		masses[BodyPart::left_hip].set(135.3,-16.49,0.29,-4.75);
		masses[BodyPart::left_thigh].set(397.98,1.31,2.01,-53.86);
		masses[BodyPart::left_tibia].set(297.06,4.71,2.1,-48.91);
		masses[BodyPart::left_ankle].set(138.92,1.42,0.28,6.38);
		masses[BodyPart::left_foot].set(163.04,24.89,3.3,-32.08);
		masses[BodyPart::left_bottom_foot].set(0,0,0,0);
		masses[BodyPart::right_pelvis].set(72.44,-7.17,11.87,27.05);
		masses[BodyPart::right_hip].set(135.3,-16.49,-0.29,-4.75);
		masses[BodyPart::right_thigh].set(397.98,1.31,-2.01,-53.86);
		masses[BodyPart::right_tibia].set(297.06,4.71,-2.1,-48.91);
		masses[BodyPart::right_ankle].set(138.92,1.42,-0.28,6.38);
		masses[BodyPart::right_foot].set(163.04,24.89,-3.3,-32.08);
		masses[BodyPart::right_bottom_foot].set(0,0,0,0);
		masses[BodyPart::torso].set(1026.28,-4.80,0.06,127.27); //z=85+42.27
  }
};
#endif
