#include "ObjectsGL.h"
#include <common/Field.h>

#include <iostream>
using namespace std;

void ObjectsGL::drawGreenCarpet() {
  glDisable(GL_LIGHTING);
  basicGL.colorRGB(0,100,0);
  auto 
    v0 = Vector3<float>(HALF_GRASS_X,HALF_GRASS_Y,-1),
    v1 = Vector3<float>(-HALF_GRASS_X,HALF_GRASS_Y,-1),
    v2 = Vector3<float>(-HALF_GRASS_X,-HALF_GRASS_Y,-1),
    v3 = Vector3<float>(HALF_GRASS_X,-HALF_GRASS_Y,-1)
  ;
  basicGL.drawSurface(v0, v1, v2, v3);
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawFieldLine(Point2D start, Point2D end) {
  glDisable(GL_LIGHTING);
  basicGL.colorRGB(Colors::White);
  float height = 2;
  auto 
    v0 = Vector3<float>(start.x-LINE_WIDTH/2,start.y-LINE_WIDTH/2,height),
    v1 = Vector3<float>(end.x+LINE_WIDTH/2,end.y-LINE_WIDTH/2,height),
    v2 = Vector3<float>(end.x+LINE_WIDTH/2,end.y+LINE_WIDTH/2,height),
    v3 = Vector3<float>(start.x-LINE_WIDTH/2,start.y+LINE_WIDTH/2,height)
  ;
  basicGL.drawSurface(v0, v1, v2, v3);
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawIntersection(Point2D p, float alpha) {
  glDisable(GL_LIGHTING);
  glPushMatrix();
  basicGL.colorRGBAlpha(Colors::Pink,alpha);  
  basicGL.translate(p,0.0);
  basicGL.drawSphere(BALL_RADIUS);
  glPopMatrix();;
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawPenaltyCross(Point2D p, float alpha){
  glDisable(GL_LIGHTING);
  auto p1 = p; p1.x -= 50;
  auto p2 = p; p2.x += 50;
  auto p3 = p; p3.y -= 50;
  auto p4 = p; p4.y += 50;
  drawFieldLine(p1, p2);
  drawFieldLine(p3, p4);
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawCenterCircle(Point2D p, float alpha){
  glDisable(GL_LIGHTING);
  glPushMatrix();
  basicGL.useFieldLineWidth();
  basicGL.colorRGB(Colors::White);
  basicGL.colorRGBAlpha(Colors::White,alpha);  
  basicGL.translate(p,0.0);
  basicGL.drawCircle(CIRCLE_RADIUS);
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawLinePoint(Point2D p, float alpha) {
  glDisable(GL_LIGHTING);
  glPushMatrix();
  basicGL.colorRGBAlpha(Colors::White,alpha);  
  basicGL.translate(p,0.0);
  basicGL.drawSphere(BALL_RADIUS);
  glPopMatrix();;
  glEnable(GL_LIGHTING);
}

void ObjectsGL::drawBall(Point2D p, float alpha) {
  drawBallColor(p,alpha,Colors::Orange);
}

void ObjectsGL::drawBallColor(Point2D p, float alpha, RGB color){
  glPushMatrix();
  basicGL.colorRGBAlpha(color,alpha);
  basicGL.translate(p,BALL_RADIUS);
  basicGL.drawSphere(BALL_RADIUS);
  glPopMatrix();
}
  

void ObjectsGL::drawBallVel(Point2D p, Vector2D vel, float alpha) {
  drawBallVelColor(p, vel, alpha, Colors::Red);
}

void ObjectsGL::drawBallVelColor(Point2D p, Vector2D vel, float alpha, RGB color) {
  glPushMatrix();
  basicGL.translate(0,0,25);
  basicGL.colorRGBAlpha(color,alpha);
  basicGL.setLineWidth(50);
  basicGL.drawLine(p, p+vel);
  glPopMatrix();
}

void ObjectsGL::drawGoal(Point2D p1, float alpha) {
  basicGL.colorRGBAlpha(Colors::Blue, alpha);
  drawGoal(p1);
}

void ObjectsGL::drawPost(Point2D p1, float alpha) {
  basicGL.colorRGBAlpha(Colors::Blue, alpha);
  drawGoalPost(p1);
}

void ObjectsGL::drawGoalPost(Point2D p) {
 glPushMatrix();  
 basicGL.translate(p);
 basicGL.drawCylinder(50.0f,GOAL_HEIGHT);
 glPopMatrix();  
}

void ObjectsGL::drawCrossBar(Point2D goalCenter) {
 glPushMatrix();  

 basicGL.translate(goalCenter,0.95*GOAL_HEIGHT);
 basicGL.rotateXDeg(90.0);
 basicGL.drawCylinder(30.0f,GOAL_Y/2.0);
 basicGL.rotateXDeg(-180.0);
 basicGL.drawCylinder(39.0f,GOAL_Y/2.0);
  
 glPopMatrix();  
}

void ObjectsGL::drawGoal(Point2D goalCenter) { 
  Point2D post;
  post.x=goalCenter.x;
  post.y=goalCenter.y+GOAL_Y/2.0;
  drawGoalPost(post); //left

  post.y=goalCenter.y-GOAL_Y/2.0;
  drawGoalPost(post); //right

  drawCrossBar(goalCenter);
}

void ObjectsGL::drawBeacon(Point2D p, RGB topColor, RGB bottomColor, float alpha) {
  basicGL.colorRGBAlpha(topColor, alpha);
  basicGL.drawCylinder(p.x, p.y, 300, p.x, p.y, 400, 110);
  basicGL.colorRGBAlpha(bottomColor, alpha);
  basicGL.drawCylinder(p.x, p.y, 200, p.x, p.y, 300, 110);
  basicGL.colorRGBAlpha(Colors::White, alpha);
  basicGL.drawCylinder(p.x, p.y, 0, p.x, p.y, 200, 110);
}

