#include "LocalizationGL.h"

using namespace Eigen;

void LocalizationGL::drawUncertaintyEllipse(Point2D loc, Point2D sd) {
  glDisable(GL_LIGHTING);
  glPushMatrix();

  basicGL.setLineWidth(3.0);

  basicGL.translate(loc,50);
  basicGL.drawEllipse(sd);

  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void LocalizationGL::drawUncertaintyEllipse(Point2D loc, Matrix2f cov) {
  glPushMatrix();
  basicGL.setLineWidth(3.0);
  basicGL.translate(loc.x, loc.y, 0);
  basicGL.drawEllipse(cov);
  glPopMatrix();
}
void LocalizationGL::drawUncertaintyEllipse(Pose2D pose, Matrix2f cov) {
  Point2D loc(pose.translation.x, pose.translation.y);
  drawUncertaintyEllipse(loc, cov);
}

void LocalizationGL::drawUncertaintyAngle(Pose2D pose, double var) {
  Point2D p(pose.translation.x, pose.translation.y);
  drawUncertaintyAngle(p, pose.rotation, sqrt(var));
}

void LocalizationGL::drawUncertaintyAngle(Point2D loc, double orientation, double sdOri) {
  glDisable(GL_LIGHTING);
  glPushMatrix();

  basicGL.translate(loc,15);
  basicGL.rotateZ(orientation);
  basicGL.colorRGBAlpha(Colors::Pink,1.0);
  basicGL.setLineWidth(8.0);

  float length = 300.0;

  // rotate left by sdOri
  basicGL.rotateZ(sdOri);

  Point2D start(0.0,0.0);
  Point2D end(length,0.0);
  basicGL.drawLine(start,end);

  // and rotate the other way
  basicGL.rotateZ(-2.0*sdOri);
  basicGL.drawLine(start,end);

  glPopMatrix();
  glEnable(GL_LIGHTING);
}


// Only drawn in 2d at the moment
void LocalizationGL::drawRelativeObjects(WorldObjectBlock* gtObjects, WorldObjectBlock* beliefObjects, RobotStateBlock* robotState) {
  if (beliefObjects == NULL || robotState == NULL){
    //cout << "no wo or robotstate to draw seen objects" << endl;
    return;
  }

  WorldObject* wo;
  WorldObject* self = &(gtObjects->objects_[robotState->WO_SELF]);

  for (int i = 0; i < NUM_WORLD_OBJS; i++){
    wo = &(beliefObjects->objects_[i]);
    // if seen, get vision dist and bearing
    if (wo->seen){
      // use vision distance and bearing to figure out image loc
      AngRad orient = (self->orientation + wo->visionBearing);
      Point2D obsLocFd (wo->visionDistance, orient, POLAR);
      obsLocFd += self->loc;
      Vector3<float> start(self->loc.x, self->loc.y, 250);
      Vector3<float> end(obsLocFd.x, obsLocFd.y,250);
      if (wo->isOwnGoal() && wo->isGoalPost()) {
        drawObservationLine(start,end,Colors::Blue);
        objectsGL.drawPost(obsLocFd,0.25);
      } else if (wo->isOwnGoal() && !wo->isGoalPost()) {
        drawObservationLine(start,end,Colors::Blue);
        objectsGL.drawGoal(obsLocFd,0.25);
      } else if (wo->isBall()) {
        drawObservationLine(start,end,Colors::Orange);
        objectsGL.drawBall(obsLocFd,0.5);
      } else if (wo->isBeacon()) {
        vector<RGB> colors;
        switch(wo->type) {
          case WO_BEACON_BLUE_YELLOW: colors = { Colors::Blue, Colors::Yellow }; break;
          case WO_BEACON_YELLOW_BLUE: colors = { Colors::Yellow, Colors::Blue }; break;
          case WO_BEACON_BLUE_PINK: colors = { Colors::Blue, Colors::Pink }; break;
          case WO_BEACON_PINK_BLUE: colors = { Colors::Pink, Colors::Blue }; break;
          case WO_BEACON_PINK_YELLOW: colors = { Colors::Pink, Colors::Yellow }; break;
          case WO_BEACON_YELLOW_PINK: colors = { Colors::Yellow, Colors::Pink }; break;
        }
        drawObservationLine(start,end,Colors::White);
        objectsGL.drawBeacon(obsLocFd, colors[0], colors[1],0.25);
      /*
      } else if (wo->isIntersection() && !wo->isUnknownIntersection()){
        drawObservationLine(start,end,Colors::White);
        objectsGL.drawIntersection(obsLocFd,0.5);
      } else if (wo->isLine() || wo->isUnknownLine()) {
        Point2D closest = wo->visionLine.relativeToGlobal(self->loc, self->orientation).getPointOnSegmentClosestTo(self->loc);
        drawObservationLine(start,Vector3<float>(closest.x, closest.y, 250),Colors::White);
        objectsGL.drawLinePoint(closest,0.5);
        //draw the line segment
        // unknown - black
        if (wo->isUnknownLine())
          basicGL.colorRGBAlpha(Colors::Black,1.0);
        // known - red
        else
          basicGL.colorRGBAlpha(Colors::Red,1.0);
        Point2D sP=wo->visionPt1;
        Point2D eP=wo->visionPt2;
        sP=sP.relativeToGlobal(self->loc,self->orientation);
        eP=eP.relativeToGlobal(self->loc,self->orientation);
        basicGL.drawLine(sP,eP,2.0);
      } else if (wo->isUnknownIntersection()) {
        drawObservationLine(start,end,Colors::Pink);
        objectsGL.drawIntersection(obsLocFd,0.5);
      } else if (wo->isCenterCircle()){
        // draw center cirlce
        drawObservationLine(start,end,Colors::White);
        objectsGL.drawCenterCircle(obsLocFd,0.5);
      } else if (wo->isUnknownPenaltyCross() || wo->isKnownPenaltyCross()){
        // draw penalty cross
        drawObservationLine(start,end,Colors::White);
        objectsGL.drawPenaltyCross(obsLocFd,0.5);
      */
      }
    }
  }
}

void LocalizationGL::drawRelativeObjectUncerts(WorldObjectBlock* gtObjects, WorldObjectBlock* beliefObjects, RobotStateBlock* robotState, LocalizationBlock* localization) {
  vector<WorldObjectType> validTypes = {
    WO_UNKNOWN_PENALTY_CROSS,
    WO_UNKNOWN_GOAL, WO_UNKNOWN_LEFT_GOALPOST, WO_UNKNOWN_RIGHT_GOALPOST, WO_UNKNOWN_GOALPOST,
    WO_UNKNOWN_L_1, WO_UNKNOWN_L_2, WO_UNKNOWN_T_1, WO_UNKNOWN_T_2,
    WO_UNKNOWN_FIELD_LINE_1, WO_UNKNOWN_FIELD_LINE_2, WO_UNKNOWN_FIELD_LINE_3, WO_UNKNOWN_FIELD_LINE_4,
    WO_CENTER_LINE, WO_CENTER_CIRCLE, WO_BALL
  };
  /*
  WorldObject& self = gtObjects->objects_[robotState->WO_SELF];
  for(auto type : validTypes) {
    auto& wobj = beliefObjects->objects_[type];
    auto uncert = localization->objectCov[type] * 10000;
    AngRad orient = (self.orientation + wobj.visionBearing);
    Point2D loc = Point2D(wobj.visionDistance, orient, POLAR) + self.loc;
    if(wobj.isLine())
      loc = wobj.visionLine.relativeToGlobal(self.loc, self.orientation).getPointOnSegmentClosestTo(self.loc);
    if(wobj.seen) {
      drawUncertaintyEllipse(loc, uncert);
    }
  }
  */
}

void LocalizationGL::drawObservationLine(Vector3<float> origin, Vector3<float> end, RGB color) {
  glDisable(GL_LIGHTING);
  glPushMatrix();
  basicGL.setLineWidth(50);
  basicGL.colorRGBAlpha(color,0.25);
  basicGL.drawLine(origin,end);
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

void LocalizationGL::drawParticles(const std::vector<Particle>& particles) {
  for(const auto& p : particles) {
    auto start = Point2D(p.x,p.y);
    auto end = start + Point2D::getPointFromPolar(100, p.t);
    basicGL.drawArrow(start, end, Colors::Indigo, Colors::Red, p.w, 5);
  }
}

void LocalizationGL::drawOdometry(Point2D loc, AngRad ori, OdometryBlock* odometry){
  glDisable(GL_LIGHTING);
  glPushMatrix();
  basicGL.translate(loc,15);
  basicGL.rotateZ(ori);
  basicGL.setLineWidth(30);
  basicGL.colorRGBAlpha(Colors::Blue,1.0);

  Vector2<float> start(0,0);
  basicGL.drawLine(start,odometry->displacement.translation * 25);

  // draw from translational odom, or from fwd?
  AngRad heading = 0; //odometry->displacement.getDirection() *RAD_T_DEG;
  // draw angular odom?
  float disp = odometry->displacement.rotation;
  if(disp < 0)
    basicGL.drawArc(heading + disp * 25, heading, 200);
  else
    basicGL.drawArc(heading, heading + disp * 25, 200);

  glPopMatrix();
  glEnable(GL_LIGHTING);
}
