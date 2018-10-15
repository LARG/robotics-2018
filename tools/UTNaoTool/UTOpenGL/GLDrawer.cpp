#include <tool/UTOpenGL/GLDrawer.h>
#include <common/annotations/AnnotationGroup.h>
#include <common/annotations/LocalizationAnnotation.h>
#include <memory/BehaviorBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/SimTruthDataBlock.h>
#include <memory/ProcessedSonarBlock.h>
#include <memory/OpponentBlock.h>
#include <memory/TeamPacketsBlock.h>
#include <memory/JointBlock.h>
#include <memory/GameStateBlock.h>

using namespace Eigen;

GLDrawer::GLDrawer(QGLWidget* parent) : 
    parent_(parent), 
    teammate(WO_TEAM_COACH),
    annotations_(NULL) {
  kickGridSize = 100.0f;
}

void GLDrawer::setGtCache(MemoryCache cache) {
  gtcache_ = cache;
}

void GLDrawer::setBeliefCache(MemoryCache cache) {
  bcache_ = cache;
}

void GLDrawer::setAnnotations(AnnotationGroup* annotations) {
  annotations_ = annotations;
}

void GLDrawer::draw(const map<DisplayOption,bool>& displayOptions) {
  display_ = displayOptions;
  if (gtcache_.world_object == NULL){
    cout << "No world objects, can not draw field" << endl;
    return;
  }

  if (display_[SHOW_FIELD]) drawField();
  overlayBasicInfoText();

  if(annotations_) drawAnnotations();

  // draw robots, ball, objects
  if (display_[SHOW_ROBOT]) drawRobot();
  if (display_[SHOW_BALL]) drawBall();
  if (display_[SHOW_RELATIVE_OBJECTS]) localizationGL.drawRelativeObjects(gtcache_.world_object, bcache_.world_object,gtcache_.robot_state);
  if (display_[SHOW_ODOMETRY]) drawOdometry();
  if (display_[SHOW_ODOMETRY_OVERLAY]) overlayOdometry();
  if (display_[SHOW_RELATIVE_OBJECT_UNCERTS]) localizationGL.drawRelativeObjectUncerts(gtcache_.world_object, bcache_.world_object, gtcache_.robot_state, bcache_.localization_mem);
  if (display_[SHOW_BEACONS]) drawBeacons();

  // truth data from sim
  if (display_[SHOW_TRUTH_ROBOT]) drawTruthRobot();
  if (display_[SHOW_TRUTH_OVERLAY]) overlayTruthText();
  if (display_[SHOW_TRUTH_BALL]) drawTruthBall();

  // overlay some text
  if (display_[SHOW_OBSERVATION_TEXT_OVERLAY]) overlayObservationText();
  if (display_[SHOW_LOCATION_TEXT_OVERLAY]) overlayLocationText();
  if (display_[SHOW_ALTERN_LOCATION_TEXT_OVERLAY]) overlayAlternLocationText();
  if (display_[SHOW_OBJECT_ID_TEXT_OVERLAY]) overlayObjectIDText();
  if (display_[SHOW_SONAR_OVERLAY]) overlaySonar();

  // opponents
  if (display_[SHOW_SEEN_OPPONENT]) drawSeenOpponents();
  if (display_[SHOW_GT_OPPONENT]) drawGtOpponents();
  if (display_[SHOW_FILTERED_OPPONENTS]) drawFilteredOpponents();
  if (display_[SHOW_OPPONENT_OVERLAY]) overlayOpponentText();

  // teammates
  if (display_[SHOW_TEAMMATES]) drawTeammates();

  // team packet info
  if (display_[SHOW_TEAM_PACKETS]) drawTeamPackets(teammate, false);
  if (display_[SHOW_COACH_BALL_PACKETS]) drawCoachBallPackets();
  if (display_[SHOW_TEAM_BALL_PACKETS]) drawTeamBallPackets();
  if (display_[SHOW_TEAM_OVERLAY]) overlayTeamPackets();

  // behavior
  if (display_[SHOW_TARGET_POINT]) drawTargetPoint();
  if (bcache_.behavior->useDottedPath) drawPathPoints();

  if (display_[SHOW_STATIC_KICK_REGION]) drawStaticKickRegion();
  if (display_[SHOW_LIVE_KICK_REGION]) drawLiveKickRegion();
  if (display_[SHOW_KICK_CHOICES]) drawKickChoices();
  if (display_[SHOW_KICK_CHOICE_OVERLAY]) overlayKickChoices();

  // live
  if (display_[SHOW_ALL_PACKETS]) drawAllTeamPackets();
  if (display_[SHOW_ALL_PACKETS_OVERLAY]) overlayAllTeamPackets();
}

void GLDrawer::drawField() {
  if (gtcache_.world_object == NULL){
    cout << "No world Objects, can not draw field" << endl;
    return;
  }

  objectsGL.drawGreenCarpet();
  if(display_[SHOW_GOALS]) {
    objectsGL.drawGoal(gtcache_.world_object->objects_[WO_OPP_GOAL].loc,1.0);
    objectsGL.drawGoal(gtcache_.world_object->objects_[WO_OWN_GOAL].loc,1.0);
  }
  if(display_[SHOW_LINES]) {
    for (int i = LINE_OFFSET; i < LINE_OFFSET + NUM_LINES; i++){
      WorldObject* wo = &(gtcache_.world_object->objects_[i]);
      objectsGL.drawFieldLine(wo->loc, wo->endLoc);
    }
    WorldObject* wo = &(gtcache_.world_object->objects_[WO_OPP_GOAL]);
    glColor3f(1,1,0);
    if (gtcache_.robot_state == NULL){
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OPP");
    } else if (gtcache_.robot_state->team_ == TEAM_RED) {
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OPP - BLUE");
    } else {
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OPP - RED");
    }

    wo = &(gtcache_.world_object->objects_[WO_OWN_GOAL]);
    glColor3f(1,1,0);
    if (gtcache_.robot_state == NULL){
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OWN");
    } else if (gtcache_.robot_state->team_ == TEAM_RED) {
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OWN - RED");
    } else {
      parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,"OWN - BLUE");
    }

    objectsGL.drawPenaltyCross(gtcache_.world_object->objects_[WO_OPP_PENALTY_CROSS].loc,1.0);
    objectsGL.drawPenaltyCross(gtcache_.world_object->objects_[WO_OWN_PENALTY_CROSS].loc,1.0);
    objectsGL.drawCenterCircle(gtcache_.world_object->objects_[WO_CENTER_CIRCLE].loc, 1.0);
  }
}

void GLDrawer::drawBall(){
  if (gtcache_.world_object == NULL) return;

  if (gtcache_.robot_state != NULL && gtcache_.behavior != NULL && gtcache_.robot_state->WO_SELF == KEEPER){
    // draw keeper ball
    WorldObject* self = &(gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF]);
    Point2D absBall = gtcache_.behavior->keeperRelBallPos.relativeToGlobal(self->loc, self->orientation);
    Point2D absBallVel = gtcache_.behavior->keeperRelBallVel.relativeToGlobal(Point2D(0,0), self->orientation);

    objectsGL.drawBallColor(absBall,1.0,Colors::Pink);
    objectsGL.drawBallVelColor(absBall, absBallVel, 1.0, Colors::Pink);
  }

  WorldObject* ball = &(gtcache_.world_object->objects_[WO_BALL]);
  objectsGL.drawBall(ball->loc,1.0);

  if (display_[SHOW_BALL_VEL])
    objectsGL.drawBallVel(ball->loc, ball->absVel, 1.0);
  if (display_[SHOW_BALL_UNCERT]) {
    basicGL.colorRGB(Colors::Orange);
    localizationGL.drawUncertaintyEllipse(ball->loc,ball->sd);
  }
}


void GLDrawer::drawTrueSimLocations(vector<MemoryCache> gtcaches){
  vector<RGB> colors = Colors::StandardColors;
  reverse(colors.begin(), colors.end());
  for(auto cache : gtcaches) {
    auto color = colors.back(); colors.pop_back();
    basicGL.colorRGBAlpha(color,1.0);
    WorldObject& robot = gtcache_.world_object->objects_[cache.robot_state->global_index_];
    WorldObject& ball = gtcache_.world_object->objects_[WO_BALL];

    float roll = cache.sensor->values_[angleX];
    float tilt = cache.sensor->values_[angleY];
      
    robotGL.drawTiltedRobot(robot.loc, robot.orientation, tilt, roll);
    objectsGL.drawBallColor(ball.loc,1.0, color);
  }
}

void GLDrawer::drawGtOpponents() {
  for(int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++) {
    auto& opp = gtcache_.world_object->objects_[i];
    basicGL.colorRGBAlpha(Colors::Red,1.0);
    robotGL.drawTiltedRobot(opp.loc, opp.orientation, 0, 0);
  }
}

void GLDrawer::drawSimRobots(vector<MemoryCache> caches) {
  if (gtcache_.world_object == NULL) return;
  
  // go through all robots
  for(auto cache : caches) {

    int i = cache.robot_state->global_index_;
    float roll = cache.sensor->values_[angleX];
    float tilt = cache.sensor->values_[angleY];
    if(i == simControl) {
      basicGL.colorRGBAlpha(Colors::White,1.0);
    } else if (cache.robot_state == NULL) {
      if (i <= WO_TEAM_LAST){
        basicGL.colorRGBAlpha(Colors::Blue,1.0);
      } else {
        basicGL.colorRGBAlpha(Colors::Red,1.0);
      }
    } else if (cache.robot_state->team_ == TEAM_BLUE) {
      basicGL.colorRGBAlpha(Colors::Blue,1.0);
    } else {
      basicGL.colorRGBAlpha(Colors::Red,1.0);
    }

    WorldObject* robot = &(gtcache_.world_object->objects_[i]);
    // draw robot tilt/roll
    robotGL.drawTiltedRobot(robot->loc, robot->height, robot->orientation, tilt, roll);
    if (display_[SHOW_NUMBERS]){
      parent_->renderText(robot->loc.x/FACT,robot->loc.y/FACT,600/FACT,QString::number(i));
    }
    // show roles
    if (display_[SHOW_ROLES]){
      QFont serifFont( "Courier", 16);
      serifFont.setBold(true);
      parent_->setFont(serifFont);
      if (i > WO_TEAM_LAST)
        basicGL.colorRGB(Colors::Black);
      else
        basicGL.colorRGB(Colors::Pink);
      parent_->renderText(robot->loc.x/FACT, robot->loc.y/FACT, 400/FACT,QString::number(i));
    }
    
  }
}

void GLDrawer::drawRobot(){
  if (gtcache_.robot_state == NULL){
    //cout << "no robot state, don't know which index is the robot" << endl;
    return;
  }

  WorldObject* self = &(gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF]);
  basicGL.colorRGBAlpha(Colors::White,1.0);
  //robotGL.drawKinematicsRobotWithBasicFeet(self,bodyModel->abs_parts_);
  float tilt = 0;
  float roll = 0;
  if (gtcache_.odometry != NULL && (gtcache_.odometry->getting_up_side_ != Getup::NONE || gtcache_.odometry->fall_direction_ != Fall::NONE)) {
    if (gtcache_.odometry->getting_up_side_ == Getup::BACK){
      tilt = -M_PI/2.0;
    }
    else if (gtcache_.odometry->getting_up_side_ == Getup::FRONT){
      tilt = M_PI/2.0;
    }
    else {
      if (gtcache_.odometry->fall_direction_ == Fall::LEFT){
        roll = -M_PI/2.0;
      }
      else if (gtcache_.odometry->fall_direction_ == Fall::RIGHT){
        roll = M_PI/2.0;
      }
      else if (gtcache_.odometry->fall_direction_ == Fall::BACKWARD){
        tilt = -M_PI/2.0;
      }
      else {
        tilt = M_PI/2.0;
      }
    }
  }

  robotGL.drawTiltedRobot(self->loc, self->height, self->orientation, tilt, roll);
  if (display_[SHOW_ROBOT_UNCERT]) {
    localizationGL.drawUncertaintyEllipse(self->loc,self->sd);
    localizationGL.drawUncertaintyAngle(self->loc,self->orientation,self->sdOrientation);
  }

}

void GLDrawer::drawOdometry(){
  if (gtcache_.odometry == NULL || gtcache_.robot_state == NULL){
    //    cout << "no odom or robot state" << endl;
    return;
  }

  WorldObject* self = &(gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF]);

  localizationGL.drawOdometry(self->loc,self->orientation,bcache_.odometry);

}


void GLDrawer::drawTruthRobot(){
  if (gtcache_.sim_truth == NULL){
    return;
  }
  basicGL.colorRGBAlpha(Colors::Green,1.0);
  Point2D pos(gtcache_.sim_truth->robot_pos_.translation.x, gtcache_.sim_truth->robot_pos_.translation.y);

  robotGL.drawSimpleRobot(pos, gtcache_.sim_truth->robot_pos_.rotation, false);

}

void GLDrawer::drawTruthBall(){
  if (gtcache_.sim_truth == NULL){
    return;
  }

  Point2D pos(gtcache_.sim_truth->ball_pos_.translation.x, gtcache_.sim_truth->ball_pos_.translation.y);

  objectsGL.drawBallColor(pos,1.0,Colors::Green);

}

void GLDrawer::drawAlternateRobots(vector<MemoryCache> caches) {
  vector<RGB> colors = Colors::LightColors;
  reverse(colors.begin(), colors.end());
  for(auto cache : caches) {
    auto color = colors.back(); colors.pop_back();
    auto& localization_mem = cache.localization_mem;
    auto& odometry = cache.odometry;
    auto& robot_state = cache.robot_state;
    auto& self = cache.world_object->objects_[robot_state->WO_SELF];
    if (localization_mem == NULL){
      // draw normal one since we can't draw multiple from kf mem
      drawRobot();
      drawBall();
      return;
    }
    float alpha = 1.0f;

    Point2D ball = localization_mem->getBallPosition();
    Point2D bvel = localization_mem->getBallVel();
    Matrix2f bcov = localization_mem->getBallCov();
    if(cache.robot_state->global_index_ > WO_TEAM_LAST) {
      ball = -ball;
    }

    // draw ball
    objectsGL.drawBallColor(ball, alpha, color);
    if (display_[SHOW_BALL_VEL])
      objectsGL.drawBallVelColor(ball, bvel, alpha, color);
    if (display_[SHOW_BALL_UNCERT]) {
      basicGL.colorRGBAlpha(color,alpha);
      localizationGL.drawUncertaintyEllipse(ball, bcov);
    }

    // draw particles
    localizationGL.drawParticles(localization_mem->particles);

    // draw position
    basicGL.colorRGBAlpha(color,alpha);
    robotGL.drawSimpleRobot(&self);
  }
}

void GLDrawer::drawSeenOpponents(){
  if (gtcache_.robot_state == NULL || gtcache_.world_object == NULL){
    return;
  }
  WorldObject *self = &(gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF]);

  // draw circle at opponent
  glPushMatrix();
  basicGL.translate(self->loc,15);
  basicGL.rotateZ(self->orientation);

  // draw opponent from vision
  for (int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++) {
    WorldObject *opp = &(gtcache_.world_object->objects_[i]);
    if (!opp->seen)
      continue;

    Vector3<float> start(0,0, 250);
    Vector3<float> end(cosf(opp->visionBearing) * opp->visionDistance, sinf(opp->visionBearing) * opp->visionDistance,125);
    RGB color = Colors::Blue;
    if (gtcache_.robot_state->team_ == TEAM_BLUE) color = Colors::Red;
    localizationGL.drawObservationLine(start, end, color);

    glPushMatrix();
    Point2D tempPoint;
    tempPoint.y = sinf(opp->visionBearing) * opp->visionDistance;
    tempPoint.x = cosf(opp->visionBearing) * opp->visionDistance;
    basicGL.translate(tempPoint);
    basicGL.colorRGBAlpha(color,1.0);
    basicGL.drawCircle(25);
    glPopMatrix();
  }

  if (gtcache_.sonar != NULL) {
    // draw opponent from sonar
    for (int opp = 0; opp < 3; opp++){
      float angle = 0, distance = 0;
      if (opp == 0) {
        if (!gtcache_.sonar->on_left_) {
          continue;
        } else {
          angle = M_PI / 4;
          distance = gtcache_.sonar->left_distance_ * 1000;
        }
      }
      if (opp == 1) {
        if (!gtcache_.sonar->on_center_) {
          continue;
        } else {
          angle = 0;
          distance = gtcache_.sonar->center_distance_ * 1000;
        }
      }
      if (opp == 2) {
        if (!gtcache_.sonar->on_right_) {
          continue;
        } else {
          angle = -M_PI / 4;
          distance = gtcache_.sonar->right_distance_ * 1000;
        }
      }

      glPushMatrix();
      Point2D tempPoint;
      tempPoint.y = sinf(angle) * distance;
      tempPoint.x = cosf(angle) * distance;
      basicGL.translate(tempPoint);
      basicGL.colorRGBAlpha(Colors::Violet, 1.0);
      basicGL.drawCircle(25);
      glPopMatrix();

    }
  }
  glPopMatrix();
}

void GLDrawer::drawFilteredOpponents(){
  if (gtcache_.robot_state == NULL) return;

  // get color
  RGB oppColor = Colors::Red;
  if (gtcache_.robot_state->team_ == TEAM_RED)
    oppColor = Colors::Blue;


  // draw each filtered opponent from opponent memory
  if (gtcache_.opponent_mem != NULL){
    gtcache_.opponent_mem->syncModels();
    for (int i = 0; i < MAX_OPP_MODELS_IN_MEM; i++){
      OpponentModel& model = gtcache_.opponent_mem->genModels[i];
      if(model.alpha < 0) continue;
      Point2D loc = model.loc, sd = model.sd, vel = model.vel;
      AngRad orient = vel.getDirection();
      basicGL.colorRGBAlpha(oppColor,0.9);
      robotGL.drawSimpleRobot(loc,orient,false);
      localizationGL.drawUncertaintyEllipse(loc,sd);
    }
  }
  // draw 4 filtered opps from world objects
  else if (gtcache_.world_object != NULL) {
    for (int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++){

      WorldObject* opp = &(gtcache_.world_object->objects_[i]);
      if (opp->sd.x > 500.0) continue;
      //if (opp->sd.x > 600 || opp->sd.y > 600) continue;


      if (gtcache_.robot_state->team_ == TEAM_BLUE) {
        basicGL.colorRGBAlpha(Colors::Red,0.9);
      } else {
        basicGL.colorRGBAlpha(Colors::Blue,0.9);
      }

      // robotGL.drawKinematicsRobotWithBasicFeet(opp,bodyModel->abs_parts_);
      robotGL.drawSimpleRobot(opp,false);

      if (gtcache_.robot_state->team_ == TEAM_BLUE) {
        basicGL.colorRGBAlpha(Colors::Red,0.9);
      } else {
        basicGL.colorRGBAlpha(Colors::Blue,0.9);
      }
      localizationGL.drawUncertaintyEllipse(opp->loc,opp->sd);
    }
  }
}

void GLDrawer::drawTeammates(){
}

void GLDrawer::drawVisionRanges(vector<MemoryCache> caches){
  for(auto cache : caches) {
    if(!gtcache_.world_object || !cache.robot_state || !cache.joint) return;
    const auto& self = gtcache_.world_object->objects_[cache.robot_state->global_index_];
    auto
      yaw = cache.joint->values_[HeadYaw],
      pitch = cache.joint->values_[HeadPitch];
    robotGL.drawVisionRange(self.loc, self.orientation, yaw, pitch);
  }
}

void GLDrawer::drawCoachBallPackets() {
}

void GLDrawer::drawTeamBallPackets() {
}

void GLDrawer::drawAllTeamPackets(){
}

void GLDrawer::drawTeamPackets(int id, bool white){
}

void GLDrawer::drawPathPoints(){
  basicGL.colorRGBAlpha(Colors::Yellow, 1.0);
  int num_points = bcache_.behavior->dottedPathPoints.size();
  for(int i=0; i<num_points-1; i++) {
    basicGL.drawLine(bcache_.behavior->dottedPathPoints[i],
                     bcache_.behavior->dottedPathPoints[i+1]);
  }
}

void GLDrawer::drawTargetPoint(){
  if (bcache_.behavior == NULL || bcache_.robot_state == NULL) return;

  // draw the target point of the robot
  if (bcache_.behavior->useTarget) {

    // draw green circle at target point
    glPushMatrix();
    if(bcache_.behavior->useAbsTarget)
      basicGL.translate(bcache_.behavior->absTargetPt);
    else {
      WorldObject* self = &(gtcache_.world_object->objects_[bcache_.robot_state->WO_SELF]);
      basicGL.translate(self->loc,15);
      basicGL.rotateZ(self->orientation);
      basicGL.translate(bcache_.behavior->targetPt);
    }
    basicGL.colorRGBAlpha(Colors::Pink,1.0);
    basicGL.drawCircle(65);
    glPopMatrix();

    // draw the target bearing
    if (bcache_.behavior->useTargetBearing) {
      glPushMatrix();
      Point2D bearingPoint = Point2D::getPointFromPolar(250, bcache_.behavior->targetBearing);
      basicGL.colorRGBAlpha(Colors::Pink,1.0);
      if(bcache_.behavior->useAbsTarget)
        basicGL.drawLine(bcache_.behavior->absTargetPt, bcache_.behavior->absTargetPt + bearingPoint);
      else {
        WorldObject* self = &(gtcache_.world_object->objects_[bcache_.robot_state->WO_SELF]);
        basicGL.translate(self->loc,15);
        basicGL.rotateZ(self->orientation);
        basicGL.drawLine(bcache_.behavior->targetPt, bcache_.behavior->targetPt + bearingPoint);
      }
      glPopMatrix();
    }

    if (bcache_.behavior->useTargetArc) {
      glPushMatrix();
      basicGL.colorRGBAlpha(Colors::LightViolet, 1.0);
      basicGL.translate(bcache_.behavior->targetArcPt);
      basicGL.drawArc(bcache_.behavior->targetArcStart, bcache_.behavior->targetArcEnd, bcache_.behavior->targetArcRadius);
      glPopMatrix();
    }
  }
}

void GLDrawer::drawLiveKickRegion(){
}

void GLDrawer::drawStaticKickRegion(){
  if (gtcache_.behavior == NULL) return;

  float halfX = KICK_REGION_SIZE_X / 2.0;
  float halfY = KICK_REGION_SIZE_Y / 2.0;

  for (int xInd = 0; xInd < NUM_KICK_REGIONS_X; xInd++){
    float xCoord = xInd * KICK_REGION_SIZE_X - (FIELD_X / 2.0);
    for (int yInd = 0; yInd < NUM_KICK_REGIONS_Y; yInd++){
      float yCoord = yInd * KICK_REGION_SIZE_Y - (FIELD_Y / 2.0);

      // get value
      bool valid = gtcache_.behavior->validKickRegion[xInd][yInd];

      if (valid){
        glPushMatrix();
        basicGL.colorRGBAlpha(Colors::White,0.33);
        basicGL.drawRectangle(Vector3<float>(xCoord-halfX,yCoord-halfY,1),Vector3<float>(xCoord+halfX,yCoord-halfY,1),Vector3<float>(xCoord+halfX,yCoord+halfY,1),Vector3<float>(xCoord-halfX,yCoord+halfY,1));
        glPopMatrix();
      }
    }
  }
}

void GLDrawer::drawKickChoices(){
  if (gtcache_.behavior == NULL || gtcache_.world_object == NULL) return;

  WorldObject* ball = &(gtcache_.world_object->objects_[WO_BALL]);
  WorldObject* self = &(gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF]);

  // draw gap we're rotating for
  glColor3f(1.0,1.0,1.0);
  basicGL.setLineWidth(25);
  basicGL.drawLine(ball->loc,ball->loc+Point2D(7000, self->orientation + gtcache_.behavior->largeGapHeading, POLAR), 0);

  // for currently selected kick
  int i = currKickChoice;
  if (currKickChoice == -1)
    i = gtcache_.behavior->kickChoice;

  // check that i is valid
  if (i < 0 || i > NUM_KICKS-1)
    return;

  KickEvaluation eval = gtcache_.behavior->kickEvaluations[i];
  if (!eval.evaluated){
    QFont serifFont( "Courier", 7);
    parent_->setFont(serifFont);
    glColor3f(1.0,1.0,1.0);
    int height=parent_->height();
    QString text;
    int y = height-37;
    int x=1;
    glColor3f(1.0,1.0,1.0);
    parent_->renderText(x,y,"Kick: "+QString::number(i));
    return;
  }

  //cout << "left: " << eval.leftPoint << " right: " << eval.rightPoint << endl;

  // draw line to left point
  if (eval.leftValid == 0){
    basicGL.colorRGB(Colors::Red);
  } else if (eval.leftValid == 1){
    basicGL.colorRGB(Colors::Blue);
  } else if (eval.leftValid == 2){
    basicGL.colorRGB(Colors::Green);
  }

  basicGL.setLineWidth(15);
  basicGL.drawLine(ball->loc,eval.leftPoint,10);
  glPushMatrix();
  basicGL.translate(eval.leftPoint,40);
  basicGL.drawCircle(25);
  glPopMatrix();

  // draw line to right point
  if (eval.rightValid > -1){
    if (eval.rightValid == 0){
      basicGL.colorRGB(Colors::Red);
    } else if (eval.rightValid == 1){
      basicGL.colorRGB(Colors::Blue);
    } else if (eval.rightValid == 2){
      basicGL.colorRGB(Colors::Green);
    }

    basicGL.setLineWidth(15);
    basicGL.drawLine(ball->loc,eval.rightPoint,10);
    glPushMatrix();
    basicGL.translate(eval.rightPoint,40);
    basicGL.drawCircle(25);
    glPopMatrix();

    // draw line to middle point, colored by opponent check
    Point2D middle = (eval.leftPoint + eval.rightPoint) / 2.0;
    if (eval.opponentBlock){
      basicGL.colorRGB(Colors::Red);
    } else {
      basicGL.colorRGB(Colors::Blue);
    }
    basicGL.setLineWidth(15);
    basicGL.drawLine(ball->loc,middle,10);
    glPushMatrix();
    basicGL.translate(middle,40);
    basicGL.drawCircle(25);
    glPopMatrix();

  }

  // draw line to weak point
  if (eval.weakValid > -1){
    if (eval.weakValid == 0){
      basicGL.colorRGB(Colors::Red);
    } else if (eval.weakValid == 1){
      basicGL.colorRGB(Colors::Blue);
    } else if (eval.weakValid == 2){
      basicGL.colorRGB(Colors::Green);
    }

    basicGL.setLineWidth(15);
    basicGL.drawLine(ball->loc,eval.weakPoint,10);
    glPushMatrix();
    basicGL.translate(eval.weakPoint,40);
    basicGL.drawCircle(25);
    glPopMatrix();
  }

  // draw line to strong point
  if (eval.strongValid > -1){
    if (eval.strongValid == 0){
      basicGL.colorRGB(Colors::Red);
    } else if (eval.strongValid == 1){
      basicGL.colorRGB(Colors::Blue);
    } else if (eval.strongValid == 2){
      basicGL.colorRGB(Colors::Green);
    }

    basicGL.setLineWidth(15);
    basicGL.drawLine(ball->loc,eval.strongPoint,10);
    glPushMatrix();
    basicGL.translate(eval.strongPoint,40);
    basicGL.drawCircle(25);
    glPopMatrix();
  }

  QFont serifFont( "Courier", 10);
  parent_->setFont(serifFont);
  if (display_[SHOW_KICK_NAME_OVERLAY]) {
    parent_->renderText(eval.leftPoint.x/FACT, eval.leftPoint.y/FACT, 100/FACT,
               QString(kickNames[i].c_str()));
  }

  serifFont = QFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  int height=parent_->height();
  QString text;
  int y = height-37;
  int x=1;
  glColor3f(1.0,1.0,1.0);

  // current kick choice, name, if its evaluated, left valid, right valid, opp
  parent_->renderText(x,y,"Kick: "+QString::number(i)+" "+QString(kickNames[i].c_str())+" Evaluated: "+QString::number(eval.evaluated));
  if (eval.evaluated)
    parent_->renderText(x,y+10,"Left: "+QString::number(eval.leftValid)+" Right: "+QString::number(eval.rightValid)+"Weak: "+QString::number(eval.weakValid)+" Strong: "+QString::number(eval.strongValid)+" Opponents: "+QString::number(eval.opponentBlock));


}

void GLDrawer::overlayKickChoices(){
  if (gtcache_.behavior == NULL) return;

  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  QString text;
  int y = 80;
  int x=1;
  glColor3f(1.0,1.0,1.0);

  // overlay info for all kicks that were evaluated
  for (int i = 0; i < NUM_KICKS; i++){

    KickEvaluation eval = gtcache_.behavior->kickEvaluations[i];
    if (!eval.evaluated) continue;

    // change color for good kick
    if (eval.leftValid && eval.rightValid && eval.weakValid && eval.strongValid && !eval.opponentBlock){
      glColor3f(0,1,0);
    } else if (eval.leftValid && eval.rightValid && eval.weakValid && eval.strongValid){
      glColor3f(0,0,1);
    } else {
      glColor3f(1,1,1);
    }

    // current kick choice, name, if its evaluated, left valid, right valid, opp
    parent_->renderText(x,y,"Kick: "+QString::number(i)+" "+QString(kickNames[i].c_str())+" Left: "+QString::number(eval.leftValid)+" Right: "+QString::number(eval.rightValid)+" Weak: "+QString::number(eval.weakValid)+" Strong: "+QString::number(eval.strongValid)+" Opponents: "+QString::number(eval.opponentBlock));
    y += 10;
  }

}


void GLDrawer::overlayAllTeamPackets(){
}

void GLDrawer::overlayTeamPackets(){
}

void GLDrawer::overlayLocationText() {
  if (gtcache_.world_object == NULL) return;

  // special overlay if we're in sim mode
  if (currentSim == 0){
    QFont serifFont( "Courier", 7);
    parent_->setFont(serifFont);
    glColor3f(1.0,1.0,1.0);
    parent_->renderText(1,10,"True Simulation Locations");
    if (simControl != 0){
      WorldObject* self=&gtcache_.world_object->objects_[simControl];
      parent_->renderText(1,20,"Robot ("+QString::number((int)self->loc.x)
                 + "," + QString::number((int)self->loc.y)
                 + "), " + QString::number((int)RAD_T_DEG*self->orientation));
    }
    WorldObject* ball=&gtcache_.world_object->objects_[WO_BALL];
    parent_->renderText(1,30,"Ball ("+QString::number((int)ball->loc.x)
               + "," + QString::number((int)ball->loc.y)+ ") ("
               +QString::number((int)ball->absVel.x) + ","
               + QString::number((int)ball->absVel.y)+ ")");
    return;
  }

  // for actual overlay, require robot state
  if (gtcache_.world_object == NULL || gtcache_.robot_state == NULL) return;

  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);

  // re-write robot pose text at bottom
  WorldObject* self=&gtcache_.world_object->objects_[gtcache_.robot_state->WO_SELF];
  parent_->renderText(1,10,"Robot ("+QString::number((int)self->loc.x)
             + "," + QString::number((int)self->loc.y)
             + "), " + QString::number((int)RAD_T_DEG*self->orientation));
  parent_->renderText(1,20,"      ("+QString::number((int)self->sd.x)
             + "," + QString::number((int)self->sd.y)
             + "), " + QString::number((int)RAD_T_DEG*self->sdOrientation));

  WorldObject* ball=&gtcache_.world_object->objects_[WO_BALL];
  float dx = ball->distance * cosf(ball->bearing);
  float dy = ball->distance * sinf(ball->bearing);

  parent_->renderText(1,30,"Ball ("+QString::number((int)ball->loc.x)
             + "," + QString::number((int)ball->loc.y)+ ") ("+QString::number((int)ball->absVel.x)
             + "," + QString::number((int)ball->absVel.y)+ ")");
  parent_->renderText(1,40,"     ("+QString::number((int)ball->sd.x)
             + "," + QString::number((int)ball->sd.y)+ ") (na,na)");
  parent_->renderText(1,50,"RelBall ("+QString::number((int)dx)+","+QString::number((int)dy)+") ("+QString::number((int)ball->relVel.x) + "," + QString::number((int)ball->relVel.y)+ ")");
}

void GLDrawer::overlayOpponentText(){
  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  int row = 70;

  // print >4 filtered opps from opponent mem
  if (gtcache_.opponent_mem != NULL){
    gtcache_.opponent_mem->syncModels();
    for (int i = 0; i < MAX_OPP_MODELS_IN_MEM; i++){
      OpponentModel model = gtcache_.opponent_mem->getModel(i);
      if (model.alpha < 0) continue;

      parent_->renderText(1,row,"Opponent ("+QString::number((int)model.loc.x)
                 + "," + QString::number((int)(model.loc.y))+ ") ("+QString::number((int)model.vel.x)
                 + "," + QString::number((int)model.vel.y)+ ")");
      parent_->renderText(1,row + 10,"     ("+QString::number((int)model.sd.x)
                 + "," + QString::number((int)model.sd.y)+ ")");
      row += 25;
    }
  }
  // print 4 filtered opps from world objects
  else {
    for (int i = WO_OPPONENT_FIRST; i <= WO_OPPONENT_LAST; i++) {
      WorldObject* opp=&gtcache_.world_object->objects_[i];

      parent_->renderText(1,row,"Opponent ("+QString::number((int)opp->loc.x)
                 + "," + QString::number((int)opp->loc.y)+ ") ("+QString::number((int)opp->absVel.x)
                 + "," + QString::number((int)opp->absVel.y)+ ")");
      parent_->renderText(1,row + 10,"     ("+QString::number((int)opp->sd.x)
                 + "," + QString::number((int)opp->sd.y)+ ")");
      row += 25;
    }
  }

  //if (visionMem != NULL)
  //parent_->renderText(1,row,"NumOppoVision: ("+QString::number(visionMem->numOppoVision)+ ")");
}

void GLDrawer::overlaySonar(){
  if (gtcache_.sonar == NULL) return;

  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  int row = 220;

  parent_->renderText(1,row += 10, "Sonar: Left: "+QString::number((int)gtcache_.sonar->on_left_) +", "+QString::number((int)(gtcache_.sonar->left_distance_*1000)));

  parent_->renderText(1,row += 10, "Sonar: Center: "+QString::number((int)gtcache_.sonar->on_center_) +", "+QString::number((int)(gtcache_.sonar->center_distance_*1000)));

  parent_->renderText(1,row += 10, "Sonar: Right: "+QString::number((int)gtcache_.sonar->on_right_) +", "+QString::number((int)(gtcache_.sonar->right_distance_*1000)));

  parent_->renderText(1, row+=10, "Bump Left: "+QString::number(gtcache_.sonar->bump_left_)+", Right: "+QString::number(gtcache_.sonar->bump_right_));

}

void GLDrawer::overlayOdometry() {
  if (gtcache_.odometry == NULL) return;

  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  int row = 270;

  parent_->renderText(1,row += 10, "Walk Odom: Fwd: "+QString::number((int)gtcache_.odometry->displacement.translation.x)+", Side: "+QString::number((int)gtcache_.odometry->displacement.translation.y)+", Turn: "+QString::number((int)(gtcache_.odometry->displacement.rotation*RAD_T_DEG)));
  if (gtcache_.odometry->didKick) {
    parent_->renderText(1,row += 10, "Did kick vel: "+QString::number(gtcache_.odometry->kickVelocity)+", Head: "+QString::number(gtcache_.odometry->kickHeading * RAD_T_DEG,'f',2));
  } else {
    parent_->renderText(1,row += 10, "No kick ");
  }
  if (gtcache_.odometry->getting_up_side_ != Getup::NONE || gtcache_.odometry->fall_direction_ != Fall::NONE){

    parent_->renderText(1,row += 10, "Fallen dir "+QString::number(gtcache_.odometry->fall_direction_)+ " getting up from " + QString::number(gtcache_.odometry->getting_up_side_));
  } else {
    parent_->renderText(1,row += 10, "Not Fallen");
  }
}

void GLDrawer::overlayAlternLocationText() {
}


void GLDrawer::overlayObservationText() {
  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);

  WorldObject *wo;
  QString text;
  int y = 10;
  int x=165;
  glColor3f(0.2,0.8,0.8);
  parent_->renderText(x,y,"Observations:");
  y+=10;
  glColor3f(1.0,1.0,1.0);
  for (int i = 0; i < NUM_WORLD_OBJS; i++){
    if(not display_[SHOW_GOALS] and i >= WO_OWN_GOAL and i <= WO_UNKNOWN_GOALPOST) {
      continue;
    }
    wo = &(bcache_.world_object->objects_[i]);
    // if seen, get distance and angle
    if (wo->seen){
      text = QString("(") + ::getName((WorldObjectType)i) +
        ", D: " + QString::number((int)wo->visionDistance) +
        ", B: " + QString::number((int)(RAD_T_DEG*wo->visionBearing)) +
        ", X: " + QString::number((int)(wo->visionDistance *cos(wo->visionBearing))) +
        ", Y: " + QString::number((int)(wo->visionDistance *sin(wo->visionBearing))) + ")";
      parent_->renderText(x,y,text);
      y+=10;
    }
  }
}

void GLDrawer::overlayTruthText() {
  if (gtcache_.sim_truth == NULL) return;

  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);

  QString text;
  int height=parent_->height();
  int x = 1;
  int y = height - 27;

  glColor3f(1.0,1.0,1.0);

  text = "True Robot: (" + QString::number(gtcache_.sim_truth->robot_pos_.translation.x) + ", " + QString::number(gtcache_.sim_truth->robot_pos_.translation.y) + "), " + QString::number(gtcache_.sim_truth->robot_pos_.rotation);
  parent_->renderText(x,y,text);

  y += 10;
  text = "True Ball: (" + QString::number(gtcache_.sim_truth->ball_pos_.translation.x) + ", " + QString::number(gtcache_.sim_truth->ball_pos_.translation.y) + ")";
  parent_->renderText(x,y,text);

}

void GLDrawer::overlayBasicInfoText() {
  QFont serifFont( "Courier", 7);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);
  int height=parent_->height();
  QString text;
  int y = height-7;
  int x=1;
  glColor3f(0.2,0.8,0.8);
  // frame #
  parent_->renderText(x,y,"Frame:");
  glColor3f(1.0,1.0,1.0);
  x+=40;
  if (gtcache_.frame_info != NULL)
    parent_->renderText(x,y,QString::number(gtcache_.frame_info->frame_id));
  else
    parent_->renderText(x,y,"?");
  x+=30;
  // play speed:
  //glColor3f(0.2,0.8,0.8);
  //parent_->renderText(x,y,"Play Speed:");
  //x += 65;
  //glColor3f(1.0,1.0,1.0);
  //parent_->renderText(x,y,QString::number(playSpeed_));
  //x += 30;
  //glColor3f(0.2,0.8,0.8);
  // robot self index
  parent_->renderText(x,y,"WO_SELF:");
  glColor3f(1.0,1.0,1.0);
  x+=50;
  if (gtcache_.robot_state != NULL)
    parent_->renderText(x,y,QString::number(gtcache_.robot_state->WO_SELF));
  else
    parent_->renderText(x,y,"?");
  glColor3f(0.2,0.8,0.8);
  x+= 20;
  // game state
  parent_->renderText(x,y,"State:");
  glColor3f(1.0,1.0,1.0);
  x+=40;
  if (gtcache_.game_state != NULL)
    parent_->renderText(x,y,stateNames[gtcache_.game_state->state()].c_str());
  else
    parent_->renderText(x,y,"?");
  glColor3f(0.2,0.8,0.8);
  x+=60;
  // role
  parent_->renderText(x,y,"Role:");
  glColor3f(1.0,1.0,1.0);
  x+=30;
  // if (gtcache_.robot_state != NULL)
  //   parent_->renderText(x,y,roleNames[gtcache_.robot_state->role_].c_str());
  // else
  //   parent_->renderText(x,y,"?");

}

void GLDrawer::overlayObjectIDText() {
  QFont serifFont( "Courier", 12);
  parent_->setFont(serifFont);
  glColor3f(0.5,0.5,0.7);
  WorldObject *wo;
  for (int i=LINE_OFFSET; i<LINE_OFFSET+NUM_LINES; i++) {
    wo = &(gtcache_.world_object->objects_[i]);
    Point2D center = (wo->loc + wo->endLoc) /2.0;
    parent_->renderText(center.x/FACT,center.y/FACT,100/FACT,QString::number(i));
    //cout << wo->lineLoc.center.x/FACT << " " << wo->lineLoc.center.y/FACT;
  }
  glColor3f(0.5,1.0,0.5);
  for (int i=INTERSECTION_OFFSET; i<INTERSECTION_OFFSET+NUM_INTERSECTIONS; i++) {
    wo = &(gtcache_.world_object->objects_[i]);
    parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,250/FACT,QString::number(i));
    //cout << wo->lineLoc.center.x/FACT << " " << wo->lineLoc.center.y/FACT;
  }
  // draw center circle and both penalty crosses
  int i = WO_CENTER_CIRCLE;
  wo = &(gtcache_.world_object->objects_[i]);
  parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,250/FACT,QString::number(i));
  for (int i = CROSS_OFFSET; i < CROSS_OFFSET+NUM_CROSSES;i++){
    wo = &(gtcache_.world_object->objects_[i]);
    parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,250/FACT,QString::number(i));
  }

  for (int i = WO_OWN_GOAL; i <= WO_OPP_RIGHT_GOALPOST; i++){
    wo = &(gtcache_.world_object->objects_[i]);
    glColor3f(1,1,0);
    parent_->renderText(wo->loc.x/FACT,wo->loc.y/FACT,1000/FACT,QString::number(i));
  }
}

void GLDrawer::displaySimInfo(string simInfo){
  // display sim info
  QFont serifFont( "Courier", 15);
  parent_->setFont(serifFont);
  glColor3f(1.0,1.0,1.0);


  glColor3f(1,0,0);
  parent_->renderText(340,95, "Red "+QString::number(gtcache_.game_state->opponentScore));
  glColor3f(0,0,1);
  parent_->renderText(200,95, "Blue "+QString::number(gtcache_.game_state->ourScore));
}

void GLDrawer::drawAnnotations() {
  if(annotations_ == NULL) return;
  float maxDiff = 250.0;
  for(auto la : annotations_->getLocalizationAnnotations()) {
    int fdiff = la->frame() - gtcache_.frame_info->frame_id;
    float fratio = max(min(fdiff / maxDiff, 1.0f), -1.0f);
    if(fratio >= 1.0f || fratio <= -1.0f) {
      continue;
    }
    int r = 255 + fratio * 255, g = (fdiff == 0 ? 255 : 0), b = 255 - fratio * 255;
    r = min(r, 255), b = min(b, 255);
    float alpha = 1.0f - abs(fratio);
    RGB color = TORGB(r,g,b);
    Point2D center = Point2D(la->pose().translation.x, la->pose().translation.y);
    Point2D offset = Point2D::getPointFromPolar(100, la->pose().rotation);
    Point2D start = center - offset, end = center + offset;
    basicGL.drawArrow(start, end, color, color, alpha, 50);
  }
}

void GLDrawer::drawBeacons() {
  if(gtcache_.world_object == NULL) return;
  map<WorldObjectType,vector<RGB>> beacons = {
    { WO_BEACON_BLUE_YELLOW, { Colors::Blue, Colors::Yellow } },
    { WO_BEACON_YELLOW_BLUE, { Colors::Yellow, Colors::Blue } },
    { WO_BEACON_BLUE_PINK, { Colors::Blue, Colors::Pink } },
    { WO_BEACON_PINK_BLUE, { Colors::Pink, Colors::Blue } },
    { WO_BEACON_PINK_YELLOW, { Colors::Pink, Colors::Yellow } },
    { WO_BEACON_YELLOW_PINK, { Colors::Yellow, Colors::Pink } }
  };
  for(auto beacon : beacons) {
    const auto& object = gtcache_.world_object->objects_[beacon.first];
    objectsGL.drawBeacon(object.loc, beacon.second[0], beacon.second[1]);
  }
}
