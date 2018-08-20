#include "RobotGL.h"
#include <common/RobotInfo.h>

#include <QGLViewer/qglviewer.h>

void RobotGL::drawStickFigure(Vector3<float> *pos,RGB body_color, RGB left_color, RGB right_color) {
  basicGL.colorRGB(Colors::White);
  // Draw joint markers
  for (int i=0; i<BodyFrame::NUM_POINTS; i++) { // Don't draw spheres for the feet
    if( i < BodyFrame::left_foot_front_left) 
      basicGL.drawSphere(pos[i], 6);
  }

  //Draw links
  basicGL.colorRGB(body_color);
  basicGL.setLineWidth(6);
  basicGL.drawSolidLine(pos[BodyFrame::left_shoulder],pos[BodyFrame::right_shoulder]);
  Vector3<float> mid_shoulder = (pos[BodyFrame::left_shoulder]+pos[BodyFrame::right_shoulder])/2.0;
  basicGL.drawSolidLine(pos[BodyFrame::head],mid_shoulder);

  basicGL.drawSolidLine(pos[BodyFrame::left_shoulder],pos[BodyFrame::left_elbow]);
  basicGL.drawSolidLine(pos[BodyFrame::left_elbow],pos[BodyFrame::left_wrist]);

  basicGL.drawSolidLine(pos[BodyFrame::right_shoulder],pos[BodyFrame::right_elbow]);
  basicGL.drawSolidLine(pos[BodyFrame::right_elbow],pos[BodyFrame::right_wrist]);


  basicGL.drawSolidLine(mid_shoulder,pos[BodyFrame::torso]);
  basicGL.drawSolidLine(pos[BodyFrame::torso],pos[BodyFrame::left_hip]);
  basicGL.drawSolidLine(pos[BodyFrame::torso],pos[BodyFrame::right_hip]);

  basicGL.drawSolidLine(pos[BodyFrame::left_hip],pos[BodyFrame::left_knee]);
  basicGL.drawSolidLine(pos[BodyFrame::left_knee],pos[BodyFrame::left_ankle]);
  basicGL.drawSolidLine(pos[BodyFrame::left_ankle],pos[BodyFrame::left_foot]);

  basicGL.drawSolidLine(pos[BodyFrame::right_hip],pos[BodyFrame::right_knee]);
  basicGL.drawSolidLine(pos[BodyFrame::right_knee],pos[BodyFrame::right_ankle]);
  basicGL.drawSolidLine(pos[BodyFrame::right_ankle],pos[BodyFrame::right_foot]);

  // Draw Head
  Vector3<float> head = pos[BodyFrame::head];
  head.z+=54;// guess
  basicGL.drawSphere(head,54);

  // Draw feet
  drawFoot(left_color,pos[BodyFrame::left_foot_front_right],pos[BodyFrame::left_foot_front_left],pos[BodyFrame::left_foot_rear_left],pos[BodyFrame::left_foot_rear_right]);

  drawFoot(right_color,pos[BodyFrame::right_foot_front_right],pos[BodyFrame::right_foot_front_left],pos[BodyFrame::right_foot_rear_left],pos[BodyFrame::right_foot_rear_right]);

}

void RobotGL::drawFoot(RGB color, Vector3<float> pt1, Vector3<float> pt2, Vector3<float> pt3, Vector3<float> pt4){
  basicGL.colorRGB(color);
  basicGL.drawPrism(pt1,pt2,pt3,pt4);
}
void RobotGL::drawFoot(RGB color, Vector2<float> pt1, Vector2<float> pt2, Vector2<float> pt3, Vector2<float> pt4, float height){
  basicGL.colorRGB(color);
  basicGL.drawRectangleAtHeight(pt1,pt2,pt3,pt4,height);
}
void RobotGL::drawFoot(RGB color, Vector3<float> pt1, Vector3<float> pt2, Vector3<float> pt3, Vector3<float> pt4, float height){
  basicGL.colorRGB(color);
  basicGL.drawRectangleAtHeight(pt1,pt2,pt3,pt4,height);
}

void RobotGL::drawCoM(Vector3<float> CoM) {
  basicGL.colorRGB(Colors::Blue);
  basicGL.drawSphere(CoM, 20);
}

void RobotGL::drawVBase(Vector3<float> vBase) {
  basicGL.colorRGBAlpha(Colors::LightOrange,0.75);
  basicGL.drawSphere(vBase, 10);
}

void RobotGL::drawGroundCoM(Vector3<float> CoM) {
  basicGL.colorRGBAlpha(Colors::Blue,0.75);
  basicGL.drawSphere(CoM, 10);
}

void RobotGL::drawFootCommand(Vector3<float> command) {
  basicGL.colorRGBAlpha(Colors::Yellow,0.75);
  basicGL.drawSphere(command, 8);
}

void RobotGL::drawSimpleRobot() {

  // two feet, cylinder and sphere head version
  float offset = 15.0;
  float forward = 75.0;
  float backward = -75.0;
  float outside = 90.0;

  // left foot
  Vector3<float> l1(forward,offset,0);
  Vector3<float> l2(forward,outside,0);
  Vector3<float> l3(backward,outside,0);
  Vector3<float> l4(backward,offset,0);
  basicGL.drawSurface(l1, l2, l3, l4);

  // right foot
  Vector3<float> r1(forward,-outside,0);
  Vector3<float> r2(forward,-offset,0);
  Vector3<float> r3(backward,-offset,0);
  Vector3<float> r4(backward,-outside,0);
  basicGL.drawSurface(r1, r2, r3, r4);


  // body cylinder
  basicGL.drawCylinder(50, 350);

  // head
  Vector3<float> headHeight;
  headHeight.z = 400.0;
  basicGL.drawSphere(headHeight, 60.0f);

  // orientation line
  basicGL.colorRGB(25,25,25);
  Vector3<float> x1(0,0,30);
  Vector3<float> x2(175,0,30);
  basicGL.drawLine(x1, x2);

  // sphere with line version
  /*
  //basicGL.colorRGB(150,150,150);
  basicGL.drawSphere(100.0f);
  basicGL.colorRGB(25,25,25);

  Vector3<float> x1(0,0,30);
  Vector3<float> x2(175,0,30);
  basicGL.drawLine(x1, x2);
  */
}

void RobotGL::drawSimpleRobot(Point2D loc, double orientation, bool fallen) {
  glPushMatrix();
  basicGL.translateRotateZ(loc,orientation);
  if (fallen) basicGL.rotateY(M_PI/2.0);
  drawSimpleRobot();
  glPopMatrix();
}

void RobotGL::drawTiltedRobot(Pose2D pose, double tilt, double roll) {
  Point2D p(pose.translation.x, pose.translation.y);
  drawTiltedRobot(p, pose.rotation, tilt, roll);
}
void RobotGL::drawTiltedRobot(Point2D loc, double height, double orient, double tilt, double roll){
  glPushMatrix();
  basicGL.translate(0,0,height);
  drawTiltedRobot(loc, orient, tilt, roll);
  glPopMatrix();
}

void RobotGL::drawTiltedRobot(Point2D loc, double orient, double tilt, double roll){
  glPushMatrix();
  basicGL.translateRotateZ(loc,orient);
  basicGL.rotateY(tilt);
  basicGL.rotateX(roll);
  drawSimpleRobot();
  glPopMatrix();
}

void RobotGL::drawSimpleRobot(WorldObject* self, bool fallen) {
  drawSimpleRobot(self->loc,self->orientation,fallen);
}

void RobotGL::drawVisionRange(Point2D loc, double orientation, double yaw, double pitch) {


  Vector3<float> origin(loc.x, loc.y, 250);

  AngRad orient1 = (orientation + yaw + FOVx/2.0);
  Point2D point1 (7000, orient1, POLAR);
  point1 += loc;
  Vector3<float> end1(point1.x, point1.y, 250);

  AngRad orient2 = (orientation + yaw - FOVx/2.0);
  Point2D point2 (7000, orient2, POLAR);
  point2 += loc;
  Vector3<float> end2(point2.x, point2.y, 250);

  glPushMatrix();
  basicGL.setLineWidth(10);
  basicGL.colorRGBAlpha(200,200,200,0.5);
  basicGL.drawLine(origin,end1);
  basicGL.drawLine(origin,end2);
  glPopMatrix();

}


/*
//  glColor3f(1.0f,1.0f,1.0f);
basicGL.setLineWidth(60);
basicGL.drawLine(pos[LEFTSHOULDER],pos[RIGHTSHOULDER]);
basicGL.drawLine(pos[LEFTSHOULDER],pos[LEFTELBOW]);
basicGL.drawLine(pos[LEFTELBOW],pos[LEFTHAND]);
basicGL.drawLine(pos[RIGHTSHOULDER],pos[RIGHTELBOW]);
basicGL.drawLine(pos[RIGHTELBOW],pos[RIGHTHAND]);
VecPosition midHip;
midHip.m_x = (pos[LEFTHIP].m_x+pos[RIGHTHIP].m_x) / 2.0;
midHip.m_y = (pos[LEFTHIP].m_y+pos[RIGHTHIP].m_y) / 2.0;
midHip.m_z = (pos[LEFTHIP].m_z+pos[RIGHTHIP].m_z) / 2.0;
basicGL.drawLine(pos[MIDDLEHEAD],midHip);
basicGL.drawLine(pos[LEFTHIP],pos[RIGHTHIP]);
basicGL.drawLine(pos[LEFTHIP],pos[LEFTKNEE]);
basicGL.drawLine(pos[LEFTKNEE],pos[LEFTANKLE]);
basicGL.drawLine(pos[LEFTANKLE],pos[LEFTFOOT]);
basicGL.drawLine(pos[RIGHTHIP],pos[RIGHTKNEE]);
basicGL.drawLine(pos[RIGHTKNEE],pos[RIGHTANKLE]);
basicGL.drawLine(pos[RIGHTANKLE],pos[RIGHTFOOT]);
VecPosition head=pos[MIDDLEHEAD];
head.m_z+=54; // guess
basicGL.drawSphere(head,54);

//Draw Cameras
glColor3f(0.0f,0.5f,1.0f);
basicGL.drawSphere(pos[BOTTOMCAMERA].m_x,pos[BOTTOMCAMERA].m_y,pos[BOTTOMCAMERA].m_z,5);

basicGL.drawSphere(pos[TOPCAMERA].m_x,pos[TOPCAMERA].m_y,pos[TOPCAMERA].m_z,5);
*/



