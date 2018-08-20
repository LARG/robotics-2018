#ifndef ROBOT_GL_H
#define ROBOT_GL_H

#include "BasicGL.h"
#include <math/Vector3.h>
#include <math/Geometry.h>
#include <common/WorldObject.h>
#include <math/Pose3D.h>
#include <math/RotationMatrix.h>
#include <common/RobotInfo.h>

class RobotGL {
public:
  
  void drawStickFigure(Vector3<float> *pos, RGB body_color, RGB left_color, RGB right_color);
  void drawCoM(Vector3<float> CoM);
  void drawGroundCoM(Vector3<float> CoM);
  void drawVBase(Vector3<float> vBase);
  void drawFoot(RGB color, Vector3<float> pt1, Vector3<float> pt2, Vector3<float> pt3, Vector3<float> pt4);
  void drawFoot(RGB color, Vector2<float> pt1, Vector2<float> pt2, Vector2<float> pt3, Vector2<float> pt4, float height);
  void drawFoot(RGB color, Vector3<float> pt1, Vector3<float> pt2, Vector3<float> pt3, Vector3<float> pt4, float height);

  void drawFootCommand(Vector3<float> command);

  void drawSimpleRobot();
  void drawSimpleRobot(Point2D loc, double angle, bool fallen = false);
  void drawSimpleRobot(WorldObject* self, bool fallen = false);
  void drawTiltedRobot(Pose2D pose, double tilt, double roll);
  void drawTiltedRobot(Point2D loc, double orient, double tilt, double roll);
  void drawTiltedRobot(Point2D loc, double height, double orient, double tilt, double roll);

  void drawVisionRange(Point2D loc, double orientation, double yaw, double pitch);
 
  //void drawBasicFeet(VecPosition* kine);
  // High level calls
  //void drawKinematicsRobotWithBasicFeet(Point2D loc, double angle, VecPosition* kine);
  //void drawKinematicsRobotWithBasicFeet(WorldObject* self, VecPosition* kine);
  
  //void drawSimpleRobot(Point2D loc, double angle);
  //void drawSimpleRobot(WorldObject* self);

  //void drawDefaultRobot(Point2D loc, double angle);
  //void drawDefaultRobot(WorldObject* self);
  

  // low level drawing
  //void drawKinematicsRobotWithBasicFeet(VecPosition* kine);
  //void drawCoMRobot(Pose3D* kine);
  //void drawBasicFeet(VecPosition* kine);
  //void drawVisionRange(Point2D loc, double orientation, double yaw, double pitch, VecPosition* kine, int camera);
  //void drawUltrasoundRange(Point2D loc, double orientation, Snapshot* current, bool filtered, bool left);

  //void drawSimpleRobot();
  //void drawDefaultRobot();

  //void drawAccel();
  BasicGL basicGL;
};

#endif


