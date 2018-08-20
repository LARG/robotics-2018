#ifndef BASIC_GL_H
#define BASIC_GL_H

#include <QGLWidget>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <common/ColorSpaces.h>
#include <math/Geometry.h>
#include <math/Vector3.h>
#include <math/Pose3D.h>

#include <common/Field.h>
#include <GL/glu.h>
#include "Colors.h"

#include <limits>

#define FACT 10

class BasicGL {
public:
  BasicGL();

  // Line drawing
  void drawLine(Point2D p1, Point2D p2);
  void drawLine(Point2D p1, Point2D p2, double z);
  void drawLine(Vector3<float> x1, Vector3<float> x2);
  void drawLine(Pose3D x1, Pose3D x2);
  void drawLine(Vector2<float> p1, Vector2<float> p2);
  
  void drawSolidLine(Point2D p1, Point2D p2);
  void drawSolidLine(Point2D p1, Point2D p2, double z);
  void drawSolidLine(Vector3<float> x1, Vector3<float> x2);
  void drawSolidLine(Pose3D x1, Pose3D x2);
  void drawSolidLine(Vector2<float> p1, Vector2<float> p2);

  // Ellipse / Circle
  void drawCircle(float radius);
  void drawEllipse(float xradius, float yradius);
  void drawEllipse(Point2D radius);
  void drawEllipse(Eigen::Matrix2f covariance);
  void drawArc(float startAngle, float endAngle, float radius);
  void drawArc(float x, float y, float z, float startAng, float endAng, float radius);

  // Shapes
  void drawRectangle(Vector3<float> x1, Vector3<float>  x2,  Vector3<float>  x3,  Vector3<float>  x4);
  void drawRectangleAtHeight(Vector2<float> x1, Vector2<float> x2, Vector2<float> x3, Vector2<float> x4, float height);
  void drawRectangleAtHeight(Vector3<float> x1, Vector3<float> x2, Vector3<float> x3, Vector3<float> x4, float height);

  void drawCylinder(float radius, float height);
  void drawCylinder(float x1, float y1, float z1, float x2, float y2, float z2, float startRadius, float endRadius=std::numeric_limits<float>::quiet_NaN(), int subdivisions=32);
  void drawSphere(float radius);
  void drawSphere(Vector3<float> x, float radius);
  void drawSphere(float x, float y, float z, float radius);
  void drawArrow(Point2D start, Point2D end);
  void drawArrow(Point2D start, Point2D end, RGB lineColor, RGB headColor, float alpha = 1.0, float width = 20);
  void drawCone(Point2D start, Point2D end, float z, float radius);

  void drawSurface(Vector3<float> v0, Vector3<float> v1, Vector3<float> v2, Vector3<float> v3);
  void drawSurface(Eigen::Vector3f v0, Eigen::Vector3f v1, Eigen::Vector3f v2, Eigen::Vector3f v3);
  Eigen::Vector3f getNormalVector(Eigen::Vector3f v1, Eigen::Vector3f v2, Eigen::Vector3f v3);

  void drawPrism(Vector3<float> v0, Vector3<float> v1, Vector3<float> v2, Vector3<float> v3);
  void drawPrism(Eigen::Vector3f v0, Eigen::Vector3f v1, Eigen::Vector3f v2, Eigen::Vector3f v3);
  // Translations / Rotations
  void translate(Point2D p);
  void translate(Point2D p,float z);
  void translate(Vector3<float> vp);
  void translateRotateZ(Point2D p, float angleRad);
  //void translateRotateZ(VecPosition vp, float angleRad);

  void translate(float x, float y, float z);
  void rotateX(float angRad);
  void rotateXDeg(float angDeg);
  void rotateY(float angRad);
  void rotateYDeg(float angDeg);
  void rotateZ(float angRad);
  void rotateZDeg(float angDeg);

  // colours and line thinkness
  void colorRGB(int r, int g, int b) { glColor3f(r/255.0,g/255.0,b/255.0); };
  void colorRGB(RGB col) { glColor3f(col.r/255.0,col.g/255.0,col.b/255.0); };    
  void colorRGBAlpha(int r, int g, int b, float alpha) { glColor4f(r/255.0,g/255.0,b/255.0,alpha); };
  void colorRGBAlpha(RGB col, float alpha) { glColor4f(col.r/255.0,col.g/255.0,col.b/255.0,alpha); };  

  void setLineWidth(float width) { glLineWidth(width/FACT); lineWidth_ = width;};
  void useFieldLineWidth() { glLineWidth(LINE_WIDTH/FACT); };

  
  GLUquadricObj* quadric;
  float lineWidth_;
};


#endif


