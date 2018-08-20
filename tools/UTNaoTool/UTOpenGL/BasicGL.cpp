#include "BasicGL.h"

using namespace Eigen;

BasicGL::BasicGL() {
  quadric=gluNewQuadric();			// Create A Pointer To The Quadric Object ( NEW )
	gluQuadricNormals(quadric, GLU_SMOOTH);	// Create Smooth Normals ( NEW )
	gluQuadricTexture(quadric, GL_TRUE);		// Create Texture Coords ( NEW )
  glEnable (GL_BLEND); 
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void BasicGL::drawArrow(Point2D start, Point2D end) {
  drawArrow(start, end, Colors::Indigo, Colors::LightIndigo);
}

void BasicGL::drawArrow(Point2D start, Point2D end, RGB lineColor, RGB headColor, float alpha, float width) {
  // Arrow height and ratio of head to line
  float height = 100;
  float headRatio = .35;

  // Draw line
  auto lstart = start, lend = end - (end - start) * headRatio;
  setLineWidth(width);
  colorRGBAlpha(lineColor, alpha);
  drawSolidLine(lstart,lend,height);

  // Draw head
  float radius = 4 * width;
  auto hstart = lend, hend = end;
  colorRGBAlpha(headColor, alpha);
  drawCone(hstart,hend,height,radius);
}

void BasicGL::drawCone(Point2D start, Point2D end, float z, float radius) {
  drawCylinder(start.x, start.y, z, end.x, end.y, z, radius, 0);
}

void BasicGL::drawLine(Point2D p1, Point2D p2) {
  drawLine(p1,p2,0);
}

void BasicGL::drawLine(Vector2<float> p1, Vector2<float> p2) {
  drawLine(Point2D(p1[0],p1[1]),Point2D(p2[0],p2[1]),0);
}

void BasicGL::drawLine(Point2D p1, Point2D p2, double z) {
  drawLine(Vector3<float>(p1.x,p1.y,z),Vector3<float>(p2.x,p2.y,z));
}

void BasicGL::drawLine(Vector3<float> x1, Vector3<float> x2) {
  glBegin(GL_LINES); 
  glVertex3f ((x1.x/FACT),(x1.y/FACT),(x1.z/FACT)); 
  glVertex3f ((x2.x/FACT),(x2.y/FACT),(x2.z/FACT)); 
  glEnd();
}

void BasicGL::drawLine(Pose3D x1, Pose3D x2) {
  drawLine(x1.translation, x2.translation);
}

void BasicGL::drawSolidLine(Point2D p1, Point2D p2) {
  drawSolidLine(p1,p2,0);
}

void BasicGL::drawSolidLine(Vector2<float> p1, Vector2<float> p2) {
  drawSolidLine(Point2D(p1[0],p1[1]),Point2D(p2[0],p2[1]),0);
}

void BasicGL::drawSolidLine(Point2D p1, Point2D p2, double z) {
  drawSolidLine(Vector3<float>(p1.x,p1.y,z),Vector3<float>(p2.x,p2.y,z));
}

void BasicGL::drawSolidLine(Vector3<float> x1, Vector3<float> x2) {
  drawCylinder(x1.x, x1.y, x1.z, x2.x, x2.y, x2.z, lineWidth_);
}

void BasicGL::drawSolidLine(Pose3D x1, Pose3D x2) {
  drawSolidLine(x1.translation, x2.translation);
}

void BasicGL::drawEllipse(Point2D radius) {
  drawEllipse(radius.x,radius.y);
}

void BasicGL::drawCircle(float radius) {
  drawEllipse(radius,radius);
}

void BasicGL::drawEllipse(float xradius, float yradius) {
  xradius/=FACT;
  yradius/=FACT;
  glBegin(GL_LINE_LOOP);
  for (int i=0; i <= 360; i++) {
    //convert degrees into radians
    float degInRad = i*DEG_T_RAD;
    glVertex3f(cos(degInRad)*xradius,sin(degInRad)*yradius,0.0f);
  } 
  glEnd();
}

void BasicGL::drawEllipse(Matrix2f covariance) {
  glDisable(GL_LIGHTING);
  glPushMatrix();
  SelfAdjointEigenSolver<Matrix2f> solver(covariance);
  if(solver.info() != Success) return;
  auto vectors = solver.eigenvectors();
  auto values = solver.eigenvalues();
  GLfloat mat[]={
    vectors(0,0), vectors(1,0), 0, 0,
    vectors(0,1), vectors(1,1), 0, 0,
               0,         0,    1, 0,
               0,         0,    0, 1,
  };
  glMultMatrixf(mat);
  drawEllipse(sqrtf(values[0]),sqrtf(values[1]));
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void BasicGL::drawArc(float startAngle, float endAngle, float radius) {
  glDisable(GL_LIGHTING);
  radius/=FACT;
  float delta = .25 * DEG_T_RAD;
  glBegin(GL_LINE_LOOP);
  startAngle = normalizeAngle(startAngle);
  endAngle = normalizeAngle(endAngle);
  float angle = startAngle;
  bool cycled = false;
  while(true) {
    angle = normalizeAngle(angle);
    if(angle <= endAngle + 1 * DEG_T_RAD) {
      cycled = true;
    }
    if(angle >= endAngle - 1 * DEG_T_RAD and cycled) {
      break;
    }
    glVertex3f(cos(angle)*radius,sin(angle)*radius,0.0f);
    angle += delta;
  }
  angle = endAngle;
  cycled = false;
  while(true) {
    angle = normalizeAngle(angle);
    if(angle >= startAngle - 1 * DEG_T_RAD) {
      cycled = true;
    }
    if(angle <= startAngle + 1 * DEG_T_RAD and cycled) {
      break;
    }
    glVertex3f(cos(angle)*radius,sin(angle)*radius,0.0f);
    angle -= delta;
  }
  glEnd();
  glEnable(GL_LIGHTING);
}


void BasicGL::drawRectangle(Vector3<float>  x1, Vector3<float>  x2,  Vector3<float>  x3,  Vector3<float>  x4) {
  glBegin(GL_POLYGON); 
  glVertex3f ((x1.x/FACT),(x1.y/FACT),(x1.z/FACT)); 
  glVertex3f ((x2.x/FACT),(x2.y/FACT),(x2.z/FACT)); 
  glVertex3f ((x3.x/FACT),(x3.y/FACT),(x3.z/FACT)); 
  glVertex3f ((x4.x/FACT),(x4.y/FACT),(x4.z/FACT)); 
  glEnd();
}

void BasicGL::drawRectangleAtHeight(Vector2<float> x1, Vector2<float> x2, Vector2<float> x3, Vector2<float> x4, float height){
  glBegin(GL_POLYGON); 
  glVertex3f ((x1.x/FACT),(x1.y/FACT),height/FACT);
  glVertex3f ((x2.x/FACT),(x2.y/FACT),height/FACT);
  glVertex3f ((x3.x/FACT),(x3.y/FACT),height/FACT);
  glVertex3f ((x4.x/FACT),(x4.y/FACT),height/FACT);
  glEnd();
}

void BasicGL::drawRectangleAtHeight(Vector3<float> x1, Vector3<float> x2, Vector3<float> x3, Vector3<float> x4, float height){
  glBegin(GL_POLYGON); 
  glVertex3f ((x1.x/FACT),(x1.y/FACT),height/FACT);
  glVertex3f ((x2.x/FACT),(x2.y/FACT),height/FACT);
  glVertex3f ((x3.x/FACT),(x3.y/FACT),height/FACT);
  glVertex3f ((x4.x/FACT),(x4.y/FACT),height/FACT);
  glEnd();
}


void BasicGL::drawCylinder(float radius, float height) {
  gluCylinder(quadric,radius/FACT,radius/FACT,height/FACT,32,32); 
}

void BasicGL::drawCylinder(float x1, float y1, float z1, float x2,float y2, float z2, float startRadius, float endRadius,int subdivisions)
{
  if(std::isnan(endRadius))
    endRadius = startRadius;
  x1 /= FACT; y1 /= FACT; z1 /= FACT;
  x2 /= FACT; y2 /= FACT; z2 /= FACT;
  startRadius /= FACT; endRadius /= FACT;
  float vx = x2-x1;
  float vy = y2-y1;
  float vz = z2-z1;

  //handle the degenerate cases
  if(vx == 0) vx = 0.0001f;
  if(vy == 0) vy = 0.0001f;
  if(vz == 0) vz = 0.0001f;
  

  float v = sqrt( vx*vx + vy*vy + vz*vz );
  float ax = 57.2957795*acos( vz/v );
  if ( vz < 0.0 )
    ax = -ax;
  float rx = -vy*vz;
  float ry = vx*vz;
  glPushMatrix();

  //draw the cylinder body
  glTranslatef( x1,y1,z1 );
  glRotatef(ax, rx, ry, 0.0);
  gluQuadricOrientation(quadric,GLU_OUTSIDE);
  gluCylinder(quadric, startRadius, endRadius, v, subdivisions, 1);

  //draw the first cap
  gluQuadricOrientation(quadric,GLU_INSIDE);
  gluDisk( quadric, 0.0, startRadius, subdivisions, 1);
  glTranslatef( 0,0,v );

  //draw the second cap
  gluQuadricOrientation(quadric,GLU_OUTSIDE);
  gluDisk( quadric, 0.0, endRadius, subdivisions, 1);
  glPopMatrix();
}

void BasicGL::drawSphere(float radius) {
  gluSphere(quadric, radius/FACT, 32, 32);
}

void BasicGL::drawSphere(Vector3<float> x,float radius) {
  glPushMatrix();
  translate(x);
  gluSphere(quadric, radius/FACT, 32, 32);
  glPopMatrix();
}


void BasicGL::drawSphere(float x, float y, float z,float radius) {
  glPushMatrix();
  translate(x,y,z);
  gluSphere(quadric, radius/FACT, 32, 32);
  glPopMatrix();
}

void BasicGL::drawArc(float x, float y, float z, float startAng, float endAng, float radius){
  glPushMatrix();
  translate(x,y,z);
  drawArc(startAng, endAng, radius);
  glPopMatrix();
}

void BasicGL::translate(Point2D p) {
  translate(p.x,p.y,0.0);
}

void BasicGL::translate(Point2D p, float z) {
  translate(p.x,p.y,z);
}

void BasicGL::translate(Vector3<float> vp) {
  translate(vp.x,vp.y,vp.z);
}

void BasicGL::translateRotateZ(Point2D p, float angleRad) {
  translate(p);
  rotateZ(angleRad);
}

//void BasicGL::translateRotateZ(VecPosition vp, float angleRad) {
//  translate(vp);
//  rotateZ(angleRad);
//}

void BasicGL::translate(float x, float y, float z) {
  glTranslatef(x/FACT,y/FACT,z/FACT);
}
 
void BasicGL::rotateX(float angRad) {
  rotateXDeg(RAD_T_DEG*angRad);	
}

void BasicGL::rotateXDeg(float angDeg) {
  glRotatef(angDeg,1.0f,0.0f,0.0f);
}

void BasicGL::rotateY(float angRad) {
  rotateYDeg(RAD_T_DEG*angRad);	
}

void BasicGL::rotateYDeg(float angDeg) {
  glRotatef(angDeg,0.0f,1.0f,0.0f);
}

void BasicGL::rotateZ(float angRad) {
  rotateZDeg(RAD_T_DEG*angRad);	
}

void BasicGL::rotateZDeg(float angDeg) {
  glRotatef(angDeg,0.0f,0.0f,1.0f);
}

Vector3f BasicGL::getNormalVector(Vector3f v1, Vector3f v2, Vector3f v3) {
  auto v = (v2 - v1).cross(v3 - v1);
  auto n = sqrt(v.dot(v.conjugate()));
  return v/n;
}

void BasicGL::drawSurface(Vector3<float> v0, Vector3<float> v1, Vector3<float> v2, Vector3<float> v3) {
  drawSurface(
    Vector3f(v0.x,v0.y,v0.z),
    Vector3f(v1.x,v1.y,v1.z),
    Vector3f(v2.x,v2.y,v2.z),
    Vector3f(v3.x,v3.y,v3.z)
  );
}

void BasicGL::drawSurface(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f v3) {
  float params[4] = {183/256.0, 65/256.0, 14/256.0, 1.0};
  v0/=FACT; v1/=FACT; v2/=FACT; v3/=FACT;
  glBegin(GL_QUADS);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, params);
  Vector3f n;
  n = getNormalVector(v0, v1, v3);
  glNormal3f(n[0], n[1], n[2]);
  glVertex3f(v0[0], v0[1], v0[2]);

  n = getNormalVector(v1, v2, v0);
  glNormal3f(n[0], n[1], n[2]);
  glVertex3f(v1[0], v1[1], v1[2]);

  n = getNormalVector(v2, v3, v1);
  glNormal3f(n[0], n[1], n[2]);
  glVertex3f(v2[0], v2[1], v2[2]);

  n = getNormalVector(v3, v0, v2);
  glNormal3f(n[0], n[1], n[2]);
  glVertex3f(v3[0], v3[1], v3[2]);
  glEnd();
}

void BasicGL::drawPrism(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f v3) {
  auto n = -10 * getNormalVector(v0, v1, v2);
  Vector3f b0 = v0 + n, b1 = v1 + n, b2 = v2 + n, b3 = v3 + n;
  drawSurface(v0, v1, v2, v3);
  drawSurface(v0, v1, v2, v3);
  drawSurface(b0, b1, v1, v0);
  drawSurface(b0, v0, v3, b3);
  drawSurface(v1, b1, b2, v2);
  drawSurface(v3, v2, b2, b3);
  drawSurface(b3, b2, b1, b0);
}

void BasicGL::drawPrism(Vector3<float> v0, Vector3<float> v1, Vector3<float> v2, Vector3<float> v3) {
  drawPrism(
    Vector3f(v0.x,v0.y,v0.z),
    Vector3f(v1.x,v1.y,v1.z),
    Vector3f(v2.x,v2.y,v2.z),
    Vector3f(v3.x,v3.y,v3.z)
  );
}
