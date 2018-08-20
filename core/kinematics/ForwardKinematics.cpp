#include "ForwardKinematics.h"

#include <stdio.h>
#define PRINT_PART(arr, part, name) \
  printf("%s x,y,z: %2.4f, %2.4f, %2.4f\n", name, arr[part].translation.x, arr[part].translation.y, arr[part].translation.z); \
  printf("%s rotX, rotY, rotZ: %2.4f, %2.4f, %2.4f\n", name, arr[part].rotation.getXAngle() * RAD_T_DEG, arr[part].rotation.getYAngle() * RAD_T_DEG, arr[part].rotation.getZAngle() * RAD_T_DEG);

void ForwardKinematics::calculateRelativePose(vector<float> joint_angles, Pose3D* rel_parts, vector<float> dimensions) {
  calculateRelativePose(&joint_angles[0], rel_parts, &dimensions[0]);
}
void ForwardKinematics::calculateRelativePose(float *joint_angles, Pose3D* rel_parts, float* dimensions) {
  calculateRelativePose(joint_angles, 0, 0, rel_parts, dimensions);
}

void ForwardKinematics::calculateRelativePose(vector<float> joint_angles, float angleXval, float angleYval, Pose3D *rel_parts, vector<float> dimensions) {
  return calculateRelativePose(&joint_angles[0], angleXval, angleYval, rel_parts, &dimensions[0]);
}
void ForwardKinematics::calculateRelativePose(float *joint_angles, float angleXval, float angleYval, Pose3D *rel_parts, float* dimensions) {
  Pose3D base = Pose3D(0,0,0)
    .rotateX(angleXval)
    .rotateY(angleYval);
  rel_parts[BodyPart::torso] = base; 
  rel_parts[BodyPart::neck] = Pose3D(rel_parts[BodyPart::torso])
    .translate(0, 0, dimensions[RobotDimensions::torsoToHeadPan])
    .rotateZ(joint_angles[HeadYaw]);
  rel_parts[BodyPart::head] = Pose3D(rel_parts[BodyPart::neck])
    .rotateY(-joint_angles[HeadPitch]);
  rel_parts[BodyPart::top_camera] = Pose3D(rel_parts[BodyPart::head])
    .translate(dimensions[RobotDimensions::xHeadTiltToTopCamera], 0, dimensions[RobotDimensions::zHeadTiltToTopCamera])
    .rotateY(dimensions[RobotDimensions::tiltOffsetToTopCamera] + dimensions[RobotDimensions::headTiltFactorTopCamera] * (M_PI_2 - abs(joint_angles[HeadPan])) / M_PI_2)
    .rotateX(dimensions[RobotDimensions::rollOffsetToTopCamera] + dimensions[RobotDimensions::headRollFactorTopCamera] * joint_angles[HeadPan])
    .rotateZ(dimensions[RobotDimensions::yawOffsetToTopCamera]);
  rel_parts[BodyPart::bottom_camera] = Pose3D(rel_parts[BodyPart::head])
    .translate(dimensions[RobotDimensions::xHeadTiltToBottomCamera], 0, dimensions[RobotDimensions::zHeadTiltToBottomCamera])
    .rotateY(dimensions[RobotDimensions::tiltOffsetToBottomCamera] + dimensions[RobotDimensions::headTiltFactorBottomCamera] * (M_PI_2 - abs(joint_angles[HeadPan])) / M_PI_2)
    .rotateX(dimensions[RobotDimensions::rollOffsetToBottomCamera] + dimensions[RobotDimensions::headRollFactorBottomCamera] * joint_angles[HeadPan])
    .rotateZ(dimensions[RobotDimensions::yawOffsetToBottomCamera]);

  
  // Body 
  for(int side = 0; side < 2; side++)
  {
    bool left = side == 0;
    int sign = left ? -1 : 1;
    // Decide on left or right arm/leg
    BodyPart::Part pelvis = left ? BodyPart::left_pelvis : BodyPart::right_pelvis;
    Joint leg0 = left ? LHipYawPitch : RHipYawPitch;
    BodyPart::Part shoulder = left ? BodyPart::left_shoulder : BodyPart::right_shoulder;
    Joint arm0 = left ? LShoulderPitch : RShoulderPitch;
    
    //Arms
    rel_parts[shoulder + 0] = Pose3D(rel_parts[BodyPart::torso])
      .translate(dimensions[RobotDimensions::armOffset1], dimensions[RobotDimensions::armOffset2] * -sign, dimensions[RobotDimensions::armOffset3])
      .rotateY(-joint_angles[arm0 + 0]);
    rel_parts[shoulder + 1] = Pose3D(rel_parts[shoulder + 0])
      .rotateZ(joint_angles[arm0 + 1] * -sign);
    rel_parts[shoulder + 2] = Pose3D(rel_parts[shoulder + 1])
      .translate(dimensions[RobotDimensions::upperArmLength], dimensions[RobotDimensions::elbowOffsetY], 0)
      .rotateX(joint_angles[arm0 + 2] * -sign);
    rel_parts[shoulder + 3] = Pose3D(rel_parts[shoulder + 2])
      .rotateZ(joint_angles[arm0 + 3] * -sign);
    // sbarrett - adding in hand
    rel_parts[shoulder + 4] = Pose3D(rel_parts[shoulder + 3])
      .translate(dimensions[RobotDimensions::lowerArmLength],0,0);
    // Legs
    rel_parts[pelvis + 0] = Pose3D(rel_parts[BodyPart::torso])
      .rotateX(dimensions[RobotDimensions::torsoHipRoll] * -sign)
      .translate(0, 0, -dimensions[RobotDimensions::torsoToHip])
      .rotateZ(joint_angles[leg0 + 0] * sign);
    rel_parts[pelvis + 1] = Pose3D(rel_parts[pelvis + 0])
      .rotateX((joint_angles[leg0 + 1] + dimensions[RobotDimensions::torsoHipRoll]) * sign);
    rel_parts[pelvis + 2] = Pose3D(rel_parts[pelvis + 1])
      .rotateY(joint_angles[leg0 + 2]);
    rel_parts[pelvis + 3] = Pose3D(rel_parts[pelvis + 2])
      .translate(0, 0, -dimensions[RobotDimensions::upperLegLength])
      .rotateY(joint_angles[leg0 + 3]);
    rel_parts[pelvis + 4] = Pose3D(rel_parts[pelvis + 3])
      .translate(0, 0, -dimensions[RobotDimensions::lowerLegLength])
      .rotateY(joint_angles[leg0 + 4]);
    rel_parts[pelvis + 5] = Pose3D(rel_parts[pelvis + 4])
      .rotateX(joint_angles[leg0 + 5] * sign);
    rel_parts[pelvis + 6] = Pose3D(rel_parts[pelvis + 5])
      .translate(0,0, -dimensions[RobotDimensions::footHeight]);
  }  
  //printf("RELATIVE:\n");
  //PRINT_PART(rel_parts, BodyPart::right_bottom_foot, "right foot");
  //PRINT_PART(rel_parts, BodyPart::left_bottom_foot, "left foot");
  //PRINT_PART(rel_parts, BodyPart::torso, "torso");
  //PRINT_PART(rel_parts, BodyPart::neck, "neck");
  //PRINT_PART(rel_parts, BodyPart::top_camera, "top cam");
  //printf("torsoToHeadPan: %2.4f\n", dimensions[RobotDimensions::torsoToHeadPan);
  //printf("torsoToHip: %2.4f\n", dimensions[RobotDimensions::torsoToHip);
  //printf("torsoToHipZ: %2.4f\n", dimensions[RobotDimensions::torsoToHipZ);
}

Pose3D ForwardKinematics::calculateVirtualBase(bool useLeft, Pose3D *rel_parts) {
  Pose3D baseFoot;
  if(useLeft)
    baseFoot = rel_parts[BodyPart::left_bottom_foot];
  else
    baseFoot = rel_parts[BodyPart::right_bottom_foot];
 
  // Get the torso in the foot's coordinate frame
  Pose3D torsoInFootFrame = rel_parts[BodyPart::torso].relativeTo(baseFoot);
  // In the foot frame, the base is offset by the torso's XY translation
  Pose3D baseInFootFrame(torsoInFootFrame.translation.x, torsoInFootFrame.translation.y, 0);
  // In the foot frame, the base is rotated by the torso's Z rotation
  baseInFootFrame.rotateZ(torsoInFootFrame.rotation.getZAngle());
  // Make the base relative to the torso
  Pose3D base = baseInFootFrame.relativeTo(torsoInFootFrame);

  return base;
}

Pose3D ForwardKinematics::calculateVirtualBase(vector<float> sensors, Pose3D *rel_parts) {
  return calculateVirtualBase(&sensors[0], rel_parts);
}

Pose3D ForwardKinematics::calculateVirtualBase(float *sensors, Pose3D *rel_parts) {
  // Choose the foot with the most force as the base
  float leftForce = 0;
  for(int i = fsrLFL; i <= fsrLRR; i++)
    leftForce += sensors[i];
  float rightForce = 0;
  for(int i = fsrRFL; i <= fsrRRR; i++)
    rightForce += sensors[i];
  Pose3D base = calculateVirtualBase(leftForce > rightForce, rel_parts);
  return base;
}

void ForwardKinematics::calculateAbsolutePose(vector<float> sensors, Pose3D *rel_parts, Pose3D *abs_parts) {
  calculateAbsolutePose(&sensors[0], rel_parts, abs_parts);
}
void ForwardKinematics::calculateAbsolutePose(float* sensors, Pose3D *rel_parts, Pose3D *abs_parts) {
  Pose3D base = calculateVirtualBase(sensors, rel_parts);
  calculateAbsolutePose(base, rel_parts, abs_parts);
}
void ForwardKinematics::calculateAbsolutePose(Pose3D base, Pose3D *rel_parts, Pose3D *abs_parts) {
  //printf("ABSOLUTE:\n");
  rel_parts[BodyPart::virtual_base] = base;
  for(int i = 0; i < BodyPart::NUM_PARTS; i++)
    abs_parts[i] = rel_parts[i].relativeTo(base);
  //PRINT_PART(abs_parts, BodyPart::virtual_base, "v base");
  //PRINT_PART(abs_parts, BodyPart::right_bottom_foot, "right foot");
  //PRINT_PART(abs_parts, BodyPart::left_bottom_foot, "left foot");
  //PRINT_PART(abs_parts, BodyPart::torso, "torso");
  //PRINT_PART(abs_parts, BodyPart::neck, "neck");
  //PRINT_PART(abs_parts, BodyPart::head, "head");
  //PRINT_PART(abs_parts, BodyPart::top_camera, "top cam");
}

void ForwardKinematics::calculateCoM(Pose3D *abs_parts, Vector3<float> &center_of_mass, const MassCalibration &mass_calibration) {
  float total_mass = 0;
  center_of_mass = Vector3<float> (0,0,0);

  for(int i = 0; i < BodyPart::NUM_PARTS; i++)
  {
    const MassCalibration::MassInfo& limb(mass_calibration.masses[i]);
    total_mass += limb.mass;
    center_of_mass += (abs_parts[i] * limb.offset) * limb.mass;
  }
  center_of_mass /= total_mass;
}

TiltRoll ForwardKinematics::calculateTiltRollFromLeg(bool left, float *joint_angles, const RobotDimensions &dimensions) {
  //Pose3D origin = Pose3D(0,0,0);

  //int sign = left ? -1 : 1;
  //// Decide on left or right arm/leg
  //Joint leg0 = left ? LHipYawPitch : RHipYawPitch;
    //// Legs
  //Pose3D pelvis = Pose3D(origin)
    //.translate(0, dimensions.lengthBetweenLegs / 2.0f * -sign, 0)
    //.rotateX(-M_PI_4 * sign)
    //.rotateZ(joint_angles[leg0 + 0] * sign);
  //Pose3D hip = Pose3D(pelvis)
    //.rotateX((joint_angles[leg0 + 1] + M_PI_4) * sign);
  //Pose3D thigh = Pose3D(hip)
    //.rotateY(joint_angles[leg0 + 2]);
  //Pose3D tibia = Pose3D(thigh)
      //.translate(0, 0, -dimensions.upperLegLength)
    //.rotateY(joint_angles[leg0 + 3]);
  //Pose3D ankle = Pose3D(tibia)
    //.translate(0, 0, -dimensions.lowerLegLength)
    //.rotateY(joint_angles[leg0 + 4]);
  //Pose3D foot = Pose3D(ankle)
    //.rotateX(joint_angles[leg0 + 5] * sign);
  //Pose3D sole = Pose3D(foot)
      //.translate(0,0, -dimensions.footHeight);

  ////std::cout << left << " " << -sole.rotation.getXAngle() << " " << -sole.rotation.getYAngle() << std::endl;
  
  TiltRoll tR;
  tR.roll_ = 0;
  tR.tilt_ = 0;
  //tR.roll_ = -sole.rotation.getXAngle();
  //tR.tilt_ = -sole.rotation.getYAngle();
  return tR;
}
