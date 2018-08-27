#include <QtGui>
#include <math.h>
#include <stdlib.h> // RAND_MAX

#include "MotionGLWidget.h"

#include <common/RobotInfo.h>
#include <kinematics/ForwardKinematics.h>
#include <motion/KickParameters.h>

#include "MotionSimulation.h"

#include "PlotWindow.h"
#include "UTMainWnd.h"
#include "JointsWindow.h"
#include "LogWindow.h"

#include <MotionCore.h>


MotionGLWidget::MotionGLWidget(QWidget* pa): QGLViewer(pa)  {
  parent = pa;
  walk_engine_ = NULL;
  body_model_ = NULL;
  sensors_ = NULL;
  frame_ = NULL;
  joint_values_ = NULL;
  joint_commands_ = NULL;
  walk_request_ = NULL;
  //setStateFileName("motion.xml");
  //restoreStateFromFile();
  motion_sim_ = NULL;
  walk_param_ = NULL;
  memory_ = NULL;
  sensor_calibration_ = NULL;
  odometry_ = NULL;
  kick_request_ = NULL;
  kick_engine_ = NULL;
  kick_module_ = NULL;
  prev_kick_state_ = KickState::NONE;
  useKeyframes_ = false;
  base_ = SupportBase::TorsoBase;
}

void MotionGLWidget::init() {
  glEnable(GL_LIGHTING);
  //setSceneRadius(HALF_GRASS_Y/FACT);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPointSize(10.0);
  glLineWidth(5.0);

  //setAxisIsDrawn(true);
  setGridIsDrawn(true);

  qglviewer::Vec c(80,-80,30);
  qglviewer::Quaternion q(0.618458, 0.291192, 0.3236, 0.654217);
  camera()->setPosition(c);
  camera()->setOrientation(q);
  setSceneRadius(100.0);

  setMode(KEYFRAMEMODE);
}

void MotionGLWidget::updateMemory(MemoryFrame* mem) {
  walk_engine_ = NULL;
  body_model_ = NULL;
  sensors_ = NULL;
  frame_ = NULL;
  joint_values_ = NULL;
  joint_commands_ = NULL;
  walk_request_ = NULL;
  walk_param_ = NULL;
  sensor_calibration_ = NULL;
  odometry_ = NULL;
  kick_request_ = NULL;
  kick_engine_ = NULL;
  kick_module_ = NULL;
  kick_params_ = NULL;

  mem->getBlockByName(body_model_,"body_model",false);
  mem->getBlockByName(sensors_,"processed_sensors",false);
  mem->getBlockByName(frame_,"frame_info",false);
  mem->getBlockByName(walk_engine_,"walk_engine",false);
  mem->getBlockByName(joint_values_,"vision_joint_angles",false);
  mem->getBlockByName(joint_commands_,"processed_joint_commands",false);
  mem->getBlockByName(walk_request_,"walk_request",false);
  mem->getBlockByName(walk_param_, "walk_param",false);
  mem->getBlockByName(sensor_calibration_, "sensor_calibration",false);
  mem->getBlockByName(odometry_, "odometry",false);
  mem->getBlockByName(kick_request_, "kick_request",false);
  mem->getBlockByName(kick_engine_, "kick_engine",false);
  mem->getBlockByName(kick_module_, "kick_module",false);
  mem->getBlockByName(kick_params_, "kick_params",false);

  if (body_model_ == NULL)
    mem->getBlockByName(body_model_,"vision_body_model",false);
  if (sensors_ == NULL)
    mem->getBlockByName(sensors_,"vision_sensors",false);

  memory_ = mem;
  useKeyframes_ = false;
}


void MotionGLWidget::draw() {

  // Todd: draw stuff based on which options are on
  // we'll always want a body model

  // various versions of the stick figure based on different data
  //if (displayOptions[SHOW_BODYMODEL]) drawBodyModel(body_model_, Colors::LightOrange);
  //if (displayOptions[SHOW_JOINTVALUESMODEL]) drawBodyModelFromJointValues();
  //if (displayOptions[SHOW_JOINTCOMMANDSMODEL]) drawBodyModelFromJointCommands();
  if (useKeyframes_) drawBodyModelFromKeyframe(lastKeyframe_);
  else drawBodyModelFromJointValues();

  // step placement
  if (displayOptions[SHOW_STEPS]) drawSteps();

  // ZMP trajectories
  if (walk_engine_ != NULL){

    // assume 100 Hz unless we know its sim
    float timeInc = 1.0/100.0;
    if (frame_ != NULL && frame_->source == MEMORY_SIM)
      timeInc = 1.0/50.0;

    // show ref trajectory
    if (displayOptions[SHOW_ZMPREF]) drawZmpTrajectory(Colors::Green, walk_engine_->zmp_ref_);
    // show current
    if (displayOptions[SHOW_CURRENTZMP]) drawZmp(Colors::Orange, walk_engine_->current_state_.zmp_, 0);
    // show desired next
    if (displayOptions[SHOW_DESIREDZMP]) drawZmp(Colors::Pink, walk_engine_->desired_next_state_.zmp_, timeInc);
    // show sensed
    if (displayOptions[SHOW_SENSEDZMP]) drawZmp(Colors::Yellow, walk_engine_->sensor_zmp_, 0);

    // show state com
    if (displayOptions[SHOW_CURRENTPEN]) drawPen(Colors::Violet, walk_engine_->current_state_.pen_pos_, 0);
    // show desired next com
    if (displayOptions[SHOW_DESIREDPEN]) drawPen(Colors::Gray, walk_engine_->desired_next_state_.pen_pos_, timeInc);
  }

  // swing foot
  if (displayOptions[SHOW_SWINGFOOT])drawSwingFoot();

  // abs feet
  if (displayOptions[SHOW_ABSFEETTARGETS]) drawAbsFeetTargets();

  // target point
  if (displayOptions[SHOW_TARGETPT]) drawTargetPoint();

  // various text overlays
  if (displayOptions[SHOW_SENSORSTEXT]) textSensors();
  if (displayOptions[SHOW_WALKTEXT]) textWalk();
  if (displayOptions[SHOW_FRAMETEXT]) textFrame();
  if (displayOptions[SHOW_WALKREQUESTTEXT]) textWalkRequest();
  if (displayOptions[SHOW_STEPSTEXT]) textSteps();
  if (displayOptions[SHOW_ZMPPENTEXT]) textZmpCom();
  if (displayOptions[SHOW_FEETTEXT]) textFeet();
  if (displayOptions[SHOW_ODOMETRYTEXT]) textOdometry();

  // kick foot targets
  if (displayOptions[SHOW_KICKFOOTTARGET]) drawKickFootTarget();
  if (displayOptions[SHOW_KICKSPLINE]) drawKickSpline();

  // kick text overlays
  if (displayOptions[SHOW_KICKREQUESTTEXT]) textKickRequest();
  if (displayOptions[SHOW_KICKTARGETTEXT]) textKickFeetTargets();
  if (displayOptions[SHOW_KICKINFO]) textKickInfo();


}


void MotionGLWidget::drawSwingFoot(){
  if (walk_engine_ == NULL) return;

  Pose3D swing_foot_ = globalToDrawingFrame(walk_engine_->swing_foot_);
  drawFoot(swing_foot_,Colors::Pink);
}


void MotionGLWidget::drawFoot(Pose3D &foot, RGB &color) {
  Vector3<float> foot_position_[4];
  float ankleForward = dimensions_.values_[RobotDimensions::footLength] - dimensions_.values_[RobotDimensions::backOfFootToAnkle];
  foot_position_[0]=Pose3D(foot).translate(ankleForward,0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  foot_position_[1]=Pose3D(foot).translate(ankleForward,-0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  foot_position_[2]=Pose3D(foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],-0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  foot_position_[3]=Pose3D(foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;

  robotGL_.drawFoot(color, foot_position_[0], foot_position_[1], foot_position_[2], foot_position_[3]);
}

void MotionGLWidget::drawZmp(RGB color, Vector2<float> val, float time){
  Pose3D drawingZmp = globalToDrawingFrame(val);

  // set height based on time
  drawingZmp.translation.z = time*TIME_TO_HEIGHT;

  // plot this point
  basicGL_.colorRGBAlpha(color, 0.7);
  basicGL_.drawSphere(drawingZmp.translation, 5);

}


void MotionGLWidget::drawPen(RGB color, Vector2<float> val, float time){

  Pose3D drawingPen = globalToDrawingFrame(val);

  // set height based on time
  drawingPen.translation.z = time*TIME_TO_HEIGHT;

  // plot this point
  basicGL_.colorRGBAlpha(color, 0.7);
  basicGL_.drawSphere(drawingPen.translation, 8);

  // possible also draw at walk height??
  if (walk_param_ != NULL) {
    drawingPen.translation.z = walk_param_->walkHeight();
    // plot this point
    basicGL_.colorRGBAlpha(color, 0.7);
    basicGL_.drawSphere(drawingPen.translation, 8);
  }
}

void MotionGLWidget::drawZmpTrajectory(RGB color, RingQueue<Vector2<float>, 100> zmp){

  // draw points from current time to future
  // assume this is at 100 Hz, unless we know its sim
  float timeInc = 1.0/100.0;
  if (frame_ != NULL && frame_->source == MEMORY_SIM)
    timeInc = 1.0/50.0;

  float time = 0;
  //  cout << "zmp queue has size " << zmp.getNumberOfEntries() << endl;
  for (int i = 0; i < zmp.getNumberOfEntries(); i++){

    Pose3D zmpLoc = globalToDrawingFrame(zmp[i]);
    zmpLoc.translation.z = time*TIME_TO_HEIGHT;

    // plot this point
    basicGL_.colorRGB(color);
    basicGL_.drawSphere(zmpLoc.translation, 5);

    time += timeInc;
  }

}

void MotionGLWidget::drawSteps(){
  if (walk_engine_ == NULL) return;

  // draw next steps, colored by foot. put time above them
  drawStep(walk_engine_->step_next_,true);
  drawStep(walk_engine_->step_after_next_,false);
  drawStep(walk_engine_->step_two_after_next_,false);
}

void MotionGLWidget::drawStep(WalkEngineBlock::Step s, bool draw_on_ground){

  Pose3D step_foot_3d_ = globalToDrawingFrame(s.position_);
  Pose2D step_foot_;
  step_foot_.translation.x = step_foot_3d_.translation.x;
  step_foot_.translation.y = step_foot_3d_.translation.y;
  step_foot_.rotation = step_foot_3d_.rotation.getZAngle();

  // translate this into 4 corners of foot
  Vector2<float> foot_position_[4];
  float ankleForward = dimensions_.values_[RobotDimensions::footLength] - dimensions_.values_[RobotDimensions::backOfFootToAnkle];
  foot_position_[0]=Pose2D(step_foot_).translate(ankleForward,0.5*dimensions_.values_[RobotDimensions::footWidth]).translation;
  foot_position_[1]=Pose2D(step_foot_).translate(ankleForward,-0.5*dimensions_.values_[RobotDimensions::footWidth]).translation;
  foot_position_[2]=Pose2D(step_foot_).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],-0.5*dimensions_.values_[RobotDimensions::footWidth]).translation;
  foot_position_[3]=Pose2D(step_foot_).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*dimensions_.values_[RobotDimensions::footWidth]).translation;


  // draw in color based on foot. blue left, red right, green stand
  RGB color = Colors::Red;
  if (s.is_left_foot_){
    color = Colors::Blue;
  }
  if (s.is_stand_)
    color = Colors::Green;

  if (draw_on_ground)
    robotGL_.drawFoot(color, foot_position_[0], foot_position_[1], foot_position_[2], foot_position_[3], -0.1);

  // also draw it at height corresponding to time
  float timeDiff = (s.frame_ - frame_->frame_id) * 0.01;
  //float timeDiff = s.time_ - frame_->seconds_since_start;
  // 1 second = TIME_TO_HEIGHT cm
  float height = timeDiff * TIME_TO_HEIGHT;
  robotGL_.drawFoot(color, foot_position_[0], foot_position_[1], foot_position_[2], foot_position_[3], height);

  // render text with target time above step
  /* or not... renderText(x,y,z...) kills cpu
  QFont serifFont( "Courier", 12);
  setFont(serifFont);
  renderText(step_foot_.translation.x/FACT, step_foot_.translation.y/FACT, height/FACT,  QString::number(s.time_,'f',2));
  */
}


void MotionGLWidget::drawTargetPoint(){
  if (walk_request_ == NULL) return;

  float radius = BALL_RADIUS;
  basicGL_.colorRGBAlpha(Colors::Pink, 0.7);
  if (walk_request_->walk_to_target_ || walk_request_->rotate_around_target_)
    basicGL_.drawSphere(walk_request_->target_point_.translation.x, walk_request_->target_point_.translation.y, radius, radius);

  // lets draw an arc for rotate around point
  if (walk_request_->rotate_around_target_)
    basicGL_.drawArc(walk_request_->target_point_.translation.x, walk_request_->target_point_.translation.y, 0, 180, 180+RAD_T_DEG*walk_request_->rotate_heading_, walk_request_->rotate_distance_);

}

void MotionGLWidget::drawBodyModel(BodyModelBlock* bm_, RGB color){
  if (bm_==NULL) return;

  calculateStickFigure(bm_);

  //Draw robot
  robotGL_.drawStickFigure(stick_figure_,color,color,color);

  robotGL_.drawCoM(bm_->center_of_mass_);
  // Project CoM onto ground
  Vector3<float> groundCoM = bm_->center_of_mass_;
  groundCoM.z=0;
  robotGL_.drawGroundCoM(groundCoM);

  Vector3<float> vBase = bm_->abs_parts_[BodyPart::virtual_base].translation;
  //robotGL_.drawVBase(vBase);

  // Comment in to save a file that describes the scene
  //saveStateToFile();
}

BodyModelBlock* MotionGLWidget::getBodyModelFromJoints(vector<float> joints){
  return getBodyModelFromJoints(&joints[0]);
}

BodyModelBlock* MotionGLWidget::getBodyModelFromJoints(array<float,NUM_JOINTS> joints){
  return getBodyModelFromJoints(&joints[0]);
}

BodyModelBlock* MotionGLWidget::getBodyModelFromJoints(float *joints){
  // create a new body model to fill in
  BodyModelBlock* new_body_model_ = new BodyModelBlock();

  Pose3D base;

  // calculate a body model from the joint values
  ForwardKinematics::calculateRelativePose(joints, new_body_model_->rel_parts_.data(), &dimensions_.values_[0]);
  if(base_ == SupportBase::TorsoBase) {
    base = *new_body_model_->getRelPartPtr(BodyPart::torso);
    base.translation.z -= 300;
  }
  else if(base_ == SupportBase::LeftFoot)
    base = ForwardKinematics::calculateVirtualBase(true, new_body_model_->rel_parts_.data());
  else if(base_ == SupportBase::RightFoot)
    base = ForwardKinematics::calculateVirtualBase(false, new_body_model_->rel_parts_.data());
  else if(sensors_ == NULL)
    base = ForwardKinematics::calculateVirtualBase(true, new_body_model_->rel_parts_.data());
  else // SensorBase
    base = ForwardKinematics::calculateVirtualBase(sensors_->values_.data(), new_body_model_->rel_parts_.data());
  ForwardKinematics::calculateAbsolutePose(base, new_body_model_->rel_parts_.data(), new_body_model_->abs_parts_.data());
  ForwardKinematics::calculateCoM(new_body_model_->abs_parts_.data(), new_body_model_->center_of_mass_,mass_calibration_);

  return new_body_model_;
}

void MotionGLWidget::drawBodyModelFromJointValues(){
  if (joint_values_ == NULL) return;

  // create a new body model to fill in
  BodyModelBlock* new_body_model_ = getBodyModelFromJoints(joint_values_->values_);

  // draw the stick figure
  drawBodyModel(new_body_model_, Colors::LightOrange);

  delete new_body_model_;
}

void MotionGLWidget::drawBodyModelFromKeyframe(Keyframe keyframe){
  // create a new body model to fill in
  BodyModelBlock* new_body_model_ = getBodyModelFromJoints(keyframe.joints);

  // draw the stick figure
  drawBodyModel(new_body_model_, Colors::LightOrange);

  delete new_body_model_;
}

void MotionGLWidget::drawBodyModelFromJointCommands(){
  if (joint_commands_ == NULL) return;

  // create a new body model to fill in
  BodyModelBlock* new_body_model_ = getBodyModelFromJoints(joint_commands_->angles_);

  drawBodyModel(new_body_model_, Colors::Gray);

  delete new_body_model_;
}

void MotionGLWidget::drawKickFootTarget(){
  if (sensors_ == NULL || body_model_ == NULL || joint_values_ == NULL) return;
  if (motion_sim_->use_com_kick_ && kick_module_ == NULL) return;
  else if(kick_engine_ == NULL) return;

  BodyModelBlock* new_body_model_ = getBodyModelFromJoints(joint_values_->values_);

  Pose3D curr_foot = kick_engine_->current_ankle_;
  Pose3D target_foot = kick_engine_->target_ankle_;

  Pose3D* torso = new_body_model_->getAbsPartPtr(BodyPart::torso);


  // these feet positions are relative to the torso... but without it rotated
  // rotate them to match drawn torso rotation

  //const Vector3<float> axis(-sensors_->values_[angleX], sensors_->values_[angleY], 0);
  RotationMatrix axis(-sensors_->values_[angleX], -sensors_->values_[angleY], 0);

  // Rotate by filtered tilt and roll
  curr_foot.translation = axis * curr_foot.translation;
  curr_foot.rotation = curr_foot.rotation * axis;
  target_foot.translation = axis * target_foot.translation;
  target_foot.rotation = target_foot.rotation * axis;

  // add z torso offset
  curr_foot.translation.z += torso->translation.z;
  target_foot.translation.z += torso->translation.z;


  drawFoot(curr_foot,Colors::Blue);
  drawFoot(target_foot,Colors::Red);
}

void MotionGLWidget::drawKickSpline(bool names){
  bool recalc_splines = false;
  if (!kick_swing_spline_.isInitialized())
    recalc_splines = true;
  if ((prev_kick_state_ == KickState::STAND) && (kick_module_->state_ != KickState::STAND))
    recalc_splines = true;
  prev_kick_state_ = kick_module_->state_;


  if (recalc_splines) {
    if (kick_params_->params_.use_stance_spline)
      createKickSplines();
    else {
    }
  }
  if (!kick_swing_spline_.isInitialized())
    return;

  //static KickState::State prev_state = KickState::NONE;
  //if ((prev_state == KickState::STAND) && (kick_module_->state_ != KickState::STAND)) {
    //float offset = kick_module_->ball_dist_side_;// - 120;
    //std::cout << "OFFSET: " << kick_module_->ball_dist_side_ << std::endl;
    ////kick_swing_spline_.setOffset(4,8,offset,1); // last 1 for y dim
    ////calcKickSplinePoints();
    //kick_params_->params_.spline_swing_ys[2] += 0.33 * offset;
    //kick_params_->params_.spline_swing_ys[3] += 0.66 * offset;
    //for (int i = 4; i <= 9; i++) {
      //kick_params_->params_.spline_swing_ys[i] += offset;
    //}
    //createKickSplines();
  //}
  //prev_state = kick_module_->state_;


  float stance_height = 175;
  //float swing_side = 100;
  float point_size = 7;
  float spline_size = 2;

  Vector3<float> stance_pos;
  Vector3<float> swing_pos;

  if (!names) {
    basicGL_.colorRGBAlpha(Colors::Blue, 0.9);
    if (kick_stance_spline_.isInitialized()) {
      for (unsigned int i = 0; i < kick_stance_spline_pts_.size(); i++) {
        basicGL_.drawSphere(kick_stance_spline_pts_[i], spline_size);
      }
    }
    if (kick_swing_spline_.isInitialized()) {
      for (unsigned int i = 0; i < kick_swing_spline_pts_.size(); i++) {
        basicGL_.drawSphere(kick_swing_spline_pts_[i], spline_size);
      }
    }
  }

  int name;

  if (kick_stance_spline_.isInitialized()) {
    for (int i = 0; i < kick_params_->params_.num_stance_spline_pts; i++) {
      stance_pos.x = kick_params_->params_.spline_stance_xs[i];
      stance_pos.y = kick_params_->params_.spline_stance_ys[i];
      stance_pos.z = kick_params_->params_.spline_stance_zs[i];
      stance_pos += body_model_->getAbsPartPtr(BodyPart::torso)->translation;
      stance_pos += body_model_->getAbsPartPtr(BodyPart::right_bottom_foot)->translation - body_model_->getAbsPartPtr(BodyPart::right_ankle)->translation;

      // set z as time
      stance_pos.z = body_model_->getAbsPartPtr(BodyPart::right_bottom_foot)->translation.z + stance_height * kick_params_->params_.spline_stance_times[i] / kick_stance_spline_.totalTime();

      name = i;
      if (selectedName() == name)
        basicGL_.colorRGBAlpha(Colors::Red, 0.9);
      else
        basicGL_.colorRGBAlpha(Colors::Green, 0.9);
      if (names)
        glPushName(name);
      basicGL_.drawSphere(stance_pos, point_size);
      if (names)
        glPopName();
    }
  }

  if (kick_swing_spline_.isInitialized()) {
    for (int i = 0; i < kick_params_->params_.num_swing_spline_pts; i++) {
      swing_pos.x = kick_params_->params_.spline_swing_xs[i];
      swing_pos.y = kick_params_->params_.spline_swing_ys[i];
      swing_pos.z = kick_params_->params_.spline_swing_zs[i];

      // set some position as time for swing
      addTimeToSwingSplinePoint(swing_pos,kick_params_->params_.spline_swing_times[i] / kick_swing_spline_.totalTime());

      name = i + MAX_SPLINE_POINTS;
      if (selectedName() == name)
        basicGL_.colorRGBAlpha(Colors::Red, 0.9);
      else
        basicGL_.colorRGBAlpha(Colors::Green, 0.9);
      if (names)
        glPushName(name);
      basicGL_.drawSphere(swing_pos, point_size);
      if (names)
        glPopName();
    }
  }
}

void MotionGLWidget::addTimeToSwingSplinePoint(Vector3<float> &pt, float time_frac) {
  // set y as time
  //pt.y = 50 + 100 * time_frac;
  // set z as time
  pt.z = 150 * time_frac;
}

void MotionGLWidget::createKickSplines() {
  KickParameters *params_ = &(kick_params_->params_);
  if (params_->states[KickState::SPLINE].state_time <= 0)
    return;
  kick_swing_spline_.set(params_->num_swing_spline_pts,params_->spline_swing_times,params_->spline_swing_xs,params_->spline_swing_ys,params_->spline_swing_zs,params_->use_akima_spline);
  kick_stance_spline_.set(params_->num_stance_spline_pts,params_->spline_stance_times,params_->spline_stance_xs,params_->spline_stance_ys,params_->spline_stance_zs,params_->use_akima_spline);

  if (kick_swing_spline_.isInitialized())
    calcKickSplinePoints();
}

void MotionGLWidget::calcKickSplinePoints() {
  int num_spline_iterp_points = 300;
  float stance_height = 175;

  Vector3<float> stance_pos;
  Vector3<float> swing_pos;


  kick_stance_spline_pts_.clear();
  kick_swing_spline_pts_.clear();

  float time_inc = kick_swing_spline_.totalTime() / num_spline_iterp_points;
  //std::cout << "totalTime: " << kick_stance_spline_.totalTime() << " inc: " << time_inc << std::endl;
  float time = 0;
  for (int i = 0; i < num_spline_iterp_points; i++) {
    time += time_inc;
    if (kick_stance_spline_.isInitialized()) {
      kick_stance_spline_.calc(time,stance_pos);
      //std::cout << time << " " << stance_pos << std::endl;
      stance_pos += body_model_->getAbsPartPtr(BodyPart::torso)->translation;
      stance_pos += body_model_->getAbsPartPtr(BodyPart::right_bottom_foot)->translation - body_model_->getAbsPartPtr(BodyPart::right_ankle)->translation;

      // set z position as time for stance
      stance_pos.z = body_model_->getAbsPartPtr(BodyPart::right_bottom_foot)->translation.z + stance_height * time / kick_stance_spline_.totalTime();

      kick_stance_spline_pts_.push_back(stance_pos);
    }

    if (kick_swing_spline_.isInitialized()) {
      kick_swing_spline_.calc(time,swing_pos);
      //swing_pos += stance_pos;

      // set some position as time for swing
      addTimeToSwingSplinePoint(swing_pos,time / kick_swing_spline_.totalTime());

      kick_swing_spline_pts_.push_back(swing_pos);
    }
  }
}

void MotionGLWidget::moveSplinePoint(int x, int y, int z) {
  if (selectedName() == -1)
    return;

  float dx = 1.0;
  float dy = 1.0;
  float dz = 1.0;
  float dt = 10;
  int t;

  if (selectedName() < MAX_SPLINE_POINTS) {
    // change in z is actually change in time
    t = z;
    z = 0;
  } else {
    // change in y is actually change in time
    t = y;
    y = 0;
  }

  int ind;
  if (selectedName() < MAX_SPLINE_POINTS) {
    ind = selectedName();
    kick_params_->params_.spline_stance_times[ind] += t * dt;
    kick_params_->params_.spline_stance_xs[ind] += x * dx;
    kick_params_->params_.spline_stance_ys[ind] += y * dy;
    kick_params_->params_.spline_stance_zs[ind] += z * dz;

    float minTime = 0;
    float maxTime = 99999;

    if (ind > 0)
      minTime = kick_params_->params_.spline_stance_times[ind-1] + 10;
    if (ind < kick_params_->params_.num_stance_spline_pts - 1)
      maxTime = kick_params_->params_.spline_stance_times[ind+1] - 10;
    kick_params_->params_.spline_stance_times[ind] = crop(kick_params_->params_.spline_stance_times[ind],minTime,maxTime);
  } else {
    ind = selectedName() - MAX_SPLINE_POINTS;
    kick_params_->params_.spline_swing_times[ind] += t * dt;
    kick_params_->params_.spline_swing_xs[ind] += x * dx;
    kick_params_->params_.spline_swing_ys[ind] += y * dy;
    kick_params_->params_.spline_swing_zs[ind] += z * dz;

    float minTime = 0;
    float maxTime = 99999;

    if (ind > 0)
      minTime = kick_params_->params_.spline_swing_times[ind-1] + 10;
    if (ind < kick_params_->params_.num_swing_spline_pts - 1)
      maxTime = kick_params_->params_.spline_swing_times[ind+1] - 10;
    kick_params_->params_.spline_swing_times[ind] = crop(kick_params_->params_.spline_swing_times[ind],minTime,maxTime);
  }

  createKickSplines();
}

void MotionGLWidget::removeSplinePoint() {
  if (selectedName() == -1)
    return;
  int ind;
  if (selectedName() < MAX_SPLINE_POINTS) {
    ind = selectedName();
    for (int i = ind; i < kick_params_->params_.num_stance_spline_pts; i++) {
      kick_params_->params_.spline_stance_times[i] = kick_params_->params_.spline_stance_times[i+1];
      kick_params_->params_.spline_stance_xs[i] = kick_params_->params_.spline_stance_xs[i+1];
      kick_params_->params_.spline_stance_ys[i] = kick_params_->params_.spline_stance_ys[i+1];
      kick_params_->params_.spline_stance_zs[i] = kick_params_->params_.spline_stance_zs[i+1];
    }
    kick_params_->params_.num_stance_spline_pts--;
  } else {
    ind = selectedName() - MAX_SPLINE_POINTS;

    for (int i = ind; i < kick_params_->params_.num_swing_spline_pts; i++) {
      kick_params_->params_.spline_swing_times[i] = kick_params_->params_.spline_swing_times[i+1];
      kick_params_->params_.spline_swing_xs[i] = kick_params_->params_.spline_swing_xs[i+1];
      kick_params_->params_.spline_swing_ys[i] = kick_params_->params_.spline_swing_ys[i+1];
      kick_params_->params_.spline_swing_zs[i] = kick_params_->params_.spline_swing_zs[i+1];
    }

    kick_params_->params_.num_swing_spline_pts--;
  }

  createKickSplines();
}

  void MotionGLWidget::addSplinePoint() {
  if (selectedName() == -1)
    return;

  int ind;
  if (selectedName() < MAX_SPLINE_POINTS) {
    ind = selectedName() + 1;
    for (int i = kick_params_->params_.num_stance_spline_pts; i >= ind; i--) {
      kick_params_->params_.spline_stance_times[i] = kick_params_->params_.spline_stance_times[i-1];
      kick_params_->params_.spline_stance_xs[i] = kick_params_->params_.spline_stance_xs[i-1];
      kick_params_->params_.spline_stance_ys[i] = kick_params_->params_.spline_stance_ys[i-1];
      kick_params_->params_.spline_stance_zs[i] = kick_params_->params_.spline_stance_zs[i-1];
    }

    kick_params_->params_.num_stance_spline_pts++;

    kick_params_->params_.spline_stance_times[ind] += 10;
  } else {
    ind = selectedName() - MAX_SPLINE_POINTS + 1;

    for (int i = kick_params_->params_.num_swing_spline_pts; i >= ind; i--) {
      kick_params_->params_.spline_swing_times[i] = kick_params_->params_.spline_swing_times[i-1];
      kick_params_->params_.spline_swing_xs[i] = kick_params_->params_.spline_swing_xs[i-1];
      kick_params_->params_.spline_swing_ys[i] = kick_params_->params_.spline_swing_ys[i-1];
      kick_params_->params_.spline_swing_zs[i] = kick_params_->params_.spline_swing_zs[i-1];
    }

    kick_params_->params_.num_swing_spline_pts++;

    kick_params_->params_.spline_swing_times[ind] += 10;
  }

  setSelectedName(selectedName() + 1);
  createKickSplines();
}


void MotionGLWidget::drawWithNames() {
  drawKickSpline(true);
}

//FOOTLENGTH 160
//FOOTWIDTH  75
void MotionGLWidget::calculateStickFigure(BodyModelBlock* bm_) {
  if (bm_ == NULL) return;
  //std::cout <<  bm_->abs_parts_[BodyPart::left_ankle].translation.y << " "<<  bm_->abs_parts_[BodyPart::left_hip].translation.y << std::endl;
  stick_figure_[BodyFrame::origin]=bm_->getAbsPartPtr(BodyPart::torso)->translation;
  stick_figure_[BodyFrame::head]=bm_->getAbsPartPtr(BodyPart::head)->translation;

  // Left Arm
  stick_figure_[BodyFrame::left_shoulder]=bm_->getAbsPartPtr(BodyPart::left_shoulder)->translation;
  stick_figure_[BodyFrame::left_elbow]=bm_->getAbsPartPtr(BodyPart::left_forearm)->translation;
  Pose3D *t = bm_->getAbsPartPtr(BodyPart::left_forearm);
  stick_figure_[BodyFrame::left_wrist]=t->translate(120,0,0).translation;

  // Right Arm
  stick_figure_[BodyFrame::right_shoulder]=bm_->getAbsPartPtr(BodyPart::right_shoulder)->translation;
  stick_figure_[BodyFrame::right_elbow]=bm_->getAbsPartPtr(BodyPart::right_forearm)->translation;
  t = bm_->getAbsPartPtr(BodyPart::right_forearm);
  stick_figure_[BodyFrame::right_wrist]=t->translate(120,0,0).translation;

  stick_figure_[BodyFrame::torso]=bm_->getAbsPartPtr(BodyPart::torso)->translation;

  // Left Leg
  stick_figure_[BodyFrame::left_hip]=bm_->getAbsPartPtr(BodyPart::left_hip)->translation;
  stick_figure_[BodyFrame::left_knee]=bm_->getAbsPartPtr(BodyPart::left_tibia)->translation;
  t = bm_->getAbsPartPtr(BodyPart::left_tibia);
  stick_figure_[BodyFrame::left_ankle]=t->translate(0,0,-102.75).translation;
  Pose3D* foot = bm_->getAbsPartPtr(BodyPart::left_bottom_foot);
  stick_figure_[BodyFrame::left_foot]=foot->translation;

  float ankleForward = dimensions_.values_[RobotDimensions::footLength] - dimensions_.values_[RobotDimensions::backOfFootToAnkle];

  //Left foot
  stick_figure_[BodyFrame::left_foot_front_left]=Pose3D(*foot).translate(ankleForward,0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::left_foot_front_right]=Pose3D(*foot).translate(ankleForward,0.5*-dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::left_foot_rear_left]=Pose3D(*foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::left_foot_rear_right]=Pose3D(*foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*-dimensions_.values_[RobotDimensions::footWidth],0).translation;

  //Right Leg
  stick_figure_[BodyFrame::right_hip]=bm_->getAbsPartPtr(BodyPart::right_hip)->translation;
  stick_figure_[BodyFrame::right_knee]=bm_->getAbsPartPtr(BodyPart::right_tibia)->translation;
  t = bm_->getAbsPartPtr(BodyPart::right_tibia);
  stick_figure_[BodyFrame::right_ankle]=t->translate(0,0,-102.75).translation;
  t = bm_->getAbsPartPtr(BodyPart::right_ankle);
  foot = bm_->getAbsPartPtr(BodyPart::right_bottom_foot);
  stick_figure_[BodyFrame::right_foot]=foot->translation;
  //Right foot
  stick_figure_[BodyFrame::right_foot_front_left]=Pose3D(*foot).translate(ankleForward,0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::right_foot_front_right]=Pose3D(*foot).translate(ankleForward,0.5*-dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::right_foot_rear_left]=Pose3D(*foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*dimensions_.values_[RobotDimensions::footWidth],0).translation;
  stick_figure_[BodyFrame::right_foot_rear_right]=Pose3D(*foot).translate(-dimensions_.values_[RobotDimensions::backOfFootToAnkle],0.5*-dimensions_.values_[RobotDimensions::footWidth],0).translation;
}

QString MotionGLWidget::Vector2ToString(Vector2<float> val){
  QString ret = "(" + QString::number(val.x,'f',2) + ", " + QString::number(val.y,'f',2)+")";
  return ret;
}

QString MotionGLWidget::Vector3ToString(Vector3<float> val){
  QString ret = "(" + QString::number(val.x,'f',2) + ", " + QString::number(val.y,'f',2)+", "+QString::number(val.z,'f',2) + ")";
  return ret;
}

void MotionGLWidget::textZmpCom(){
  if (walk_engine_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);

  int y = 100;
  int x = 0.75*width();

  renderText(x,y+=15,"Current ZMP: "+Vector2ToString(walk_engine_->current_state_.zmp_));
  renderText(x,y+=15,"Next ZMP: "+Vector2ToString(walk_engine_->desired_next_state_.zmp_));
  renderText(x,y+=15,"Sensed ZMP: "+Vector2ToString(walk_engine_->sensor_zmp_));
  renderText(x,y+=15,"Control: "+Vector2ToString(walk_engine_->current_control_));
  renderText(x,y+=15,"Current PEN: "+Vector2ToString(walk_engine_->current_state_.pen_pos_));
  renderText(x,y+=15,"Next PEN: "+Vector2ToString(walk_engine_->desired_next_state_.pen_pos_));
  if (body_model_ != NULL){
    Vector2<float> com;
    com.x = body_model_->center_of_mass_.x;
    com.y = body_model_->center_of_mass_.y;
    // need to offset this to stance foot origin
    Pose3D* stance_foot_;
    stance_foot_ = body_model_->getAbsPartPtr(BodyPart::left_bottom_foot);
    if (walk_engine_->step_current_.is_left_foot_){
      stance_foot_ = body_model_->getAbsPartPtr(BodyPart::left_bottom_foot);
    } else {
      stance_foot_ = body_model_->getAbsPartPtr(BodyPart::right_bottom_foot);
    }
    com.x -= stance_foot_->translation.x;
    com.y -= stance_foot_->translation.y;
    renderText(x,y+=15,"Sensed COM: "+Vector2ToString(com));
  }
}

void MotionGLWidget::textSensors() {
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);

  if (sensor_calibration_ != NULL) {
    QString answer = "true";
    if (!sensor_calibration_->is_calibrated_) {
      answer = "false";
      glColor3f(0.7,0.4,0.4);
    }
    renderText(0.75*width(),15,"Sensors Calibrated: "+answer);
  }

  glColor3f(0.7,0.7,0.7);
  // Tilt , roll based on sensors
  if (body_model_ != NULL) {
    renderText(0.75*width(),30,"Sensor Tilt,Roll: "+QString::number(body_model_->sensors_tilt_roll_.tilt_,'f',3)+" , "+QString::number(body_model_->sensors_tilt_roll_.roll_,'f',3));
  }
  // Tilt, roll based on stance foort
  if (body_model_ !=NULL && walk_engine_!=NULL) {
    if (walk_engine_->step_current_.is_left_foot_) {
      renderText(0.75*width(),45,"Stance Tilt,Roll: "+QString::number(body_model_->left_foot_body_tilt_roll_.tilt_,'f',3)+" , "+QString::number(body_model_->left_foot_body_tilt_roll_.roll_,'f',3));
    } else {
      renderText(0.75*width(),45,"Stance Tilt,Roll: "+QString::number(body_model_->right_foot_body_tilt_roll_.tilt_,'f',3)+" , "+QString::number(body_model_->right_foot_body_tilt_roll_.roll_,'f',3));
    }
  }

  if (body_model_ != NULL)
    renderText(0.75*width(),75,"CoM (Mes): "+QString::number(body_model_->center_of_mass_[0],'f',2)+", "+QString::number(body_model_->center_of_mass_[1],'f',2)+", "+QString::number(body_model_->center_of_mass_[2],'f',2));
}


void MotionGLWidget::textWalkRequest(){
  if (walk_request_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  int height=this->height();
  int y = height-90;
  int x = 0.7*width();

  renderText(x,y+=15,    "Walk Command: "+QString(WalkRequestBlock::getName(walk_request_->motion_)));

  QString walk_type = "Abs";
  if (walk_request_->percentage_speed_)
    walk_type = "Pct";

  renderText(x,y+=15, walk_type + " Velocity: "+Vector2ToString(walk_request_->speed_.translation) +", "+ QString::number(walk_request_->speed_.rotation,'f',2));

  QString target = "Ignored";
  if (walk_request_->walk_to_target_)
    target = "Used";
  if (walk_request_->rotate_around_target_)
    target = "Rotate";
  renderText(x,y+=15, target + " Target: "+Vector2ToString(walk_request_->target_point_.translation));

  renderText(x,y+=15, "Rotate Dist: "+QString::number(walk_request_->rotate_distance_,'f',2)+" Head: "+QString::number(walk_request_->rotate_heading_*RAD_T_DEG,'f',2));

  QString kick_leg = "RIGHT";
  if (walk_request_->kick_with_left_)
    kick_leg = "LEFT";

  if (walk_request_->perform_kick_){
    renderText(x, y+=15, kick_leg + " kick dist "+ QString::number(walk_request_->kick_distance_,'f',2)+ " heading "+QString::number(walk_request_->kick_heading_*RAD_T_DEG,'f',2));
  } else {
    renderText(x, y+=15, "No kick");
  }

  renderText(x, y+=15, "Fallen Counter: " + QString::number(walk_request_->tilt_fallen_counter_) + ", "+ QString::number(walk_request_->roll_fallen_counter_));

}


void MotionGLWidget::textKickRequest(){
  if (kick_request_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  int height=this->height();
  int y = height-75;
  int x = 0.7 * width();

  renderText(x,y+=15,"Kick Request type: "+QString(kickTypeNames[kick_request_->kick_type_].c_str())+" leg: "+QString(legNames[kick_request_->kick_leg_].c_str()));

  renderText(x,y+=15,"Desired angle: "+QString::number(kick_request_->desired_angle_*RAD_T_DEG,'f',2)+" dist: "+QString::number(kick_request_->desired_distance_,'f',2));

  QString ball("SEEN");
  if (!kick_request_->ball_seen_)
    ball = "NOT SEEN";

  //renderText(x,y+=15,"Ball " + ball + " Image loc: "+QString::number(kick_request_->ball_image_center_x_,'f',2)+", "+QString::number(kick_request_->ball_image_center_y_,'f',2));
  renderText(x,y+=15,"Ball " + ball + " rel loc(fwd,side): "+QString::number(kick_request_->ball_rel_x_,'f',2)+", "+QString::number(kick_request_->ball_rel_y_,'f',2));

  if (kick_request_->kick_running_)
    renderText(x,y+=15,"Kick RUNNING");
  else
    renderText(x,y+=15,"Kick NOT running");
}

void MotionGLWidget::textKickInfo(){
  if (motion_sim_->use_com_kick_) {
    if (kick_module_ == NULL) return;
    QFont serifFont( "Courier", 9);
    setFont(serifFont);
    glColor3f(0.7,0.7,0.7);

    int y = 100;
    int x = 0.7*width();

    renderText(x,y+=15,"Kick Type: "+QString(kickTypeNames[kick_module_->kick_type_].c_str())+" leg: "+QString(legNames[kick_module_->swing_leg_].c_str()));

    // already in degrees?
    renderText(x,y+=15,"Desired angle: "+QString::number(kick_module_->desired_kick_angle_,'f',2)+" dist: "+QString::number(kick_module_->desired_kick_distance_,'f',2));

    // state,
    renderText(x,y+=15,"Kick state: "+QString(KickState::getName(kick_module_->state_).c_str()));

    //renderText(x,y+=15,"Ball dist Side: "+QString::number(kick_engine_->ball_Dist_From_Foot_Side_,'f',2) + " Front: "+QString::number(kick_engine_->ball_Dist_From_Foot_Front_,'f',2));
  } else {
    if (kick_engine_ == NULL) return;
    QFont serifFont( "Courier", 9);
    setFont(serifFont);
    glColor3f(0.7,0.7,0.7);

    int y = 100;
    int x = 0.7*width();

    renderText(x,y+=15,"Kick Type: "+QString(kickTypeNames[kick_engine_->type_].c_str())+" leg: "+QString(legNames[kick_engine_->leg_].c_str()));

    // already in degrees?
    renderText(x,y+=15,"Desired angle: "+QString::number(kick_engine_->desired_kick_angle_,'f',2)+" dist: "+QString::number(kick_engine_->desired_kick_distance_,'f',2));

    // state,
    renderText(x,y+=15,"Kick state: "+QString(kickStateNames[kick_engine_->state_].c_str()));

    renderText(x,y+=15,"Ball dist Side: "+QString::number(kick_engine_->ball_Dist_From_Foot_Side_,'f',2) + " Front: "+QString::number(kick_engine_->ball_Dist_From_Foot_Front_,'f',2));
  }

}

void MotionGLWidget::textKickFeetTargets(){
  if (motion_sim_->use_com_kick_) {
    // TODO
  } else {
    if (kick_engine_ == NULL) return;
    QFont serifFont( "Courier", 9);
    setFont(serifFont);
    glColor3f(0.7,0.7,0.7);

    int y = height()-60;
    int x = 10;

    // current ankle
    Pose3D ankle = kick_engine_->current_ankle_;
    renderText(x,y+=15,"Current Ankle " + Vector3ToString(ankle.translation) +", rot: "+ QString::number(RAD_T_DEG*ankle.rotation.getZAngle(),'f',2));

    // target ankle
    ankle = kick_engine_->target_ankle_;
    renderText(x,y+=15,"Target Ankle " + Vector3ToString(ankle.translation) +", rot: "+ QString::number(RAD_T_DEG*ankle.rotation.getZAngle(),'f',2));
  }
}

void MotionGLWidget::textSteps(){
  if (walk_engine_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);

  int y = height()-75;
  int x = 10;

  // prev step
  QString foot("RIGHT");
  if (walk_engine_->step_prev_.is_left_foot_)
    foot = "LEFT";

  Pose2D step_stance_frame_ = walk_engine_->step_prev_.position_;//.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y,"Prev Step "+foot+" at frame " + QString::number(walk_engine_->step_prev_.frame_,'i',4)+ ": "+Vector2ToString(step_stance_frame_.translation) +", "+ QString::number(RAD_T_DEG*step_stance_frame_.rotation,'f',2));
  y+= 15;

  // current step
  if (walk_engine_->step_current_.is_left_foot_)
    foot = "LEFT";
  else
    foot = "RIGHT";
  step_stance_frame_ = walk_engine_->step_current_.position_;//.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y,"Stance Step "+foot+" at frame " + QString::number(walk_engine_->step_current_.frame_,'i',0)+ ": "+Vector2ToString(step_stance_frame_.translation) +", "+ QString::number(RAD_T_DEG*step_stance_frame_.rotation,'f',2));
  y+= 15;

  // next step
  if (walk_engine_->step_next_.is_left_foot_)
    foot = "LEFT";
  else
    foot = "RIGHT";
  step_stance_frame_ = walk_engine_->step_next_.position_;//.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y,"Next Step "+foot+" at frame " + QString::number(walk_engine_->step_next_.frame_,'i',0)+ ": "+Vector2ToString(step_stance_frame_.translation) +", "+ QString::number(RAD_T_DEG*step_stance_frame_.rotation,'f',2));
  y+=15;

  //  step after next
  if (walk_engine_->step_after_next_.is_left_foot_)
    foot = "LEFT";
  else
    foot = "RIGHT";
  step_stance_frame_ = walk_engine_->step_after_next_.position_;//.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y,"Step After Next "+foot+" at frame " + QString::number(walk_engine_->step_after_next_.frame_,'i',0)+ ": "+Vector2ToString(step_stance_frame_.translation) +", "+ QString::number(RAD_T_DEG*step_stance_frame_.rotation,'f',2));
  y+=15;

  //  step two after next
  if (walk_engine_->step_two_after_next_.is_left_foot_)
    foot = "LEFT";
  else
    foot = "RIGHT";
  step_stance_frame_ = walk_engine_->step_two_after_next_.position_;//.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y,"Step Two After Next "+foot+" at frame_ " + QString::number(walk_engine_->step_two_after_next_.frame_,'i',0)+ ": "+Vector2ToString(step_stance_frame_.translation) +", "+ QString::number(RAD_T_DEG*step_stance_frame_.rotation,'f',2));
  y+=15;

}

void MotionGLWidget::textFeet() {
  if (walk_engine_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  int y = height()-135;
  int x = 10;

  Pose3D swing_stance_frame_ = walk_engine_->swing_foot_.globalToRelative(walk_engine_->global_frame_offset_);
  renderText(x,y+=15, "Swing foot (stance frame): "+Vector3ToString(swing_stance_frame_.translation) + ", r: "+QString::number(swing_stance_frame_.rotation.getZAngle()*RAD_T_DEG,'f',2));
  renderText(x,y+=15, "Left foot (torso frame): "+Vector3ToString(walk_engine_->abs_left_foot_.translation) + ", r: "+QString::number(walk_engine_->abs_left_foot_.rotation.getZAngle()*RAD_T_DEG,'f',2));
  renderText(x,y+=15, "Right foot (torso frame): "+Vector3ToString(walk_engine_->abs_right_foot_.translation) + ", r: "+QString::number(walk_engine_->abs_right_foot_.rotation.getZAngle()*RAD_T_DEG,'f',2));


}

void MotionGLWidget::drawAbsFeetTargets(){
  if (walk_engine_ == NULL || body_model_ == NULL) return;

  Pose3D left_foot = walk_engine_->abs_left_foot_;
  Pose3D right_foot = walk_engine_->abs_right_foot_;

  drawFoot(left_foot,Colors::Yellow);
  drawFoot(right_foot,Colors::Yellow);
}

void MotionGLWidget::textWalk() {
  if (walk_engine_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  float x = 0.35*width();
  float y = 0;

  renderText(x,y+=15,"Desired Step Size: "+Vector2ToString(walk_engine_->desired_step_size_.translation)+", "+QString::number(RAD_T_DEG*walk_engine_->desired_step_size_.rotation,'f',2));
}

void MotionGLWidget::textOdometry(){
  if (odometry_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  float x = 0.35*width();
  float y = 30;

  renderText(x,y+=15,"Odometry: "+Vector2ToString(odometry_->displacement.translation)+", "+QString::number(RAD_T_DEG*odometry_->displacement.rotation,'f',2));
}

void MotionGLWidget::textFrame() {
  if (frame_ == NULL) return;
  QFont serifFont( "Courier", 9);
  setFont(serifFont);
  glColor3f(0.7,0.7,0.7);
  renderText(5,15,"Frame: "+QString::number(frame_->frame_id));
  renderText(5,30,"Time: "+QString::number(frame_->seconds_since_start,'f',2));
}

void MotionGLWidget::setAllDisplayOptions(bool value) {
  for (int i=0; i<NUM_DISPLAY_OPTIONS; i++) {
    displayOptions[i]=value;
  }
}

void MotionGLWidget::setMode(int mode){
  currentMode = mode;

  if (currentMode == BODYMODELMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_BODYMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    emit modeChanged("Body Model Figure");
  } else if (currentMode == JOINTVALUESMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    emit modeChanged("Joint Values Figure");
  } else if (currentMode == JOINTCOMMANDSMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    emit modeChanged("Joint Commands Figure");
  } else if (currentMode == GETUPSIMMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    emit modeChanged("Getup Sim");
  } else if (currentMode == WALKSIMMODE) {
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    displayOptions[SHOW_STEPS] = true;
    displayOptions[SHOW_STEPSTEXT] = true;
    displayOptions[SHOW_ZMPREF] = true;
    displayOptions[SHOW_CURRENTZMP] = true;
    displayOptions[SHOW_DESIREDZMP] = true;
    displayOptions[SHOW_SENSEDZMP] = true;
    displayOptions[SHOW_CURRENTPEN] = true;
    displayOptions[SHOW_DESIREDPEN] = true;
    displayOptions[SHOW_ZMPPENTEXT] = true;
    displayOptions[SHOW_SWINGFOOT] = true;
    displayOptions[SHOW_FEETTEXT] = true;
    displayOptions[SHOW_ODOMETRYTEXT] = true;
    displayOptions[SHOW_TARGETPT] = true;
    //   displayOptions[SHOW_ABSFEETTARGETS] = true;
    emit modeChanged("Walk Simulation");
  } else if (currentMode == WALKMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_SENSORSTEXT] = true;
    displayOptions[SHOW_WALKTEXT] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_WALKREQUESTTEXT] = true;
    displayOptions[SHOW_STEPS] = true;
    displayOptions[SHOW_STEPSTEXT] = true;
    displayOptions[SHOW_ZMPREF] = true;
    displayOptions[SHOW_CURRENTZMP] = true;
    displayOptions[SHOW_DESIREDZMP] = true;
    displayOptions[SHOW_SENSEDZMP] = true;
    displayOptions[SHOW_CURRENTPEN] = true;
    displayOptions[SHOW_DESIREDPEN] = true;
    displayOptions[SHOW_ZMPPENTEXT] = true;
    displayOptions[SHOW_SWINGFOOT] = true;
    displayOptions[SHOW_FEETTEXT] = true;
    displayOptions[SHOW_ODOMETRYTEXT] = true;
    displayOptions[SHOW_TARGETPT] = true;
    //    displayOptions[SHOW_ABSFEETTARGETS] = true;
    emit modeChanged("Walk Mode");
  } else if (currentMode == KICKMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_KICKREQUESTTEXT] = true;
    displayOptions[SHOW_KICKFOOTTARGET] = true;
    displayOptions[SHOW_KICKTARGETTEXT] = true;
    displayOptions[SHOW_KICKINFO] = true;
    displayOptions[SHOW_KICKSPLINE] = true;
    //    displayOptions[SHOW_ABSFEETTARGETS] = true;
    emit modeChanged("Kick Mode");
  } else if (currentMode == KICKSIMMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_FRAMETEXT] = true;
    displayOptions[SHOW_KICKREQUESTTEXT] = true;
    displayOptions[SHOW_KICKFOOTTARGET] = true;
    displayOptions[SHOW_KICKTARGETTEXT] = true;
    displayOptions[SHOW_KICKINFO] = true;
    displayOptions[SHOW_KICKSPLINE] = true;
    //    displayOptions[SHOW_ABSFEETTARGETS] = true;
    emit modeChanged("Kick Simulation");
  } else if (currentMode == KEYFRAMEMODE){
    setAllDisplayOptions(false);
    displayOptions[SHOW_JOINTVALUESMODEL] = true;
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = true;
    displayOptions[SHOW_STEPS] = true;
    displayOptions[SHOW_ZMPREF] = true;
    displayOptions[SHOW_CURRENTZMP] = true;
    displayOptions[SHOW_DESIREDZMP] = true;
    displayOptions[SHOW_SENSEDZMP] = true;
    displayOptions[SHOW_CURRENTPEN] = true;
    displayOptions[SHOW_DESIREDPEN] = true;
    displayOptions[SHOW_SWINGFOOT] = true;
    displayOptions[SHOW_TARGETPT] = true;

  } else {
    setAllDisplayOptions(false);
    emit modeChanged("Unknown Mode");
  }
  update();
}


void MotionGLWidget::keyPressEvent(QKeyEvent *event) {

  /////////////////////////////////
  // keys that work in all modes
  switch (event->key()) {

  case Qt::Key_F1:
    setMode(BODYMODELMODE);
    break;
  case Qt::Key_F2:
    setMode(JOINTVALUESMODE);
    break;
  case Qt::Key_F3:
    setMode(JOINTCOMMANDSMODE);
    break;
  case Qt::Key_F4:
    initMotionSim(false);
    setMode(WALKSIMMODE);
    break;
  case Qt::Key_F5:
    setMode(WALKMODE);
    break;
  case Qt::Key_F6:
    setMode(KICKMODE);
    break;
  case Qt::Key_F7:
    initMotionSim(true);
    setMode(KICKSIMMODE);
    break;
  case Qt::Key_F8:
    initMotionSim(false);
    setMode(GETUPSIMMODE);
    break;

    // turn things on/off
  case Qt::Key_S:
    if ((currentMode != WALKSIMMODE) && (currentMode != KICKSIMMODE)) {
      displayOptions[SHOW_STEPS] = !displayOptions[SHOW_STEPS];
      update();
    }
    break;
  case Qt::Key_B:
    displayOptions[SHOW_BODYMODEL] = !displayOptions[SHOW_BODYMODEL];
    update();
    break;
  case Qt::Key_V:
    displayOptions[SHOW_JOINTVALUESMODEL] = !displayOptions[SHOW_JOINTVALUESMODEL];
    update();
    break;
  case Qt::Key_C:
    displayOptions[SHOW_JOINTCOMMANDSMODEL] = !displayOptions[SHOW_JOINTCOMMANDSMODEL];
    update();
    break;
  case Qt::Key_Z:
    displayOptions[SHOW_ZMPREF] = !displayOptions[SHOW_ZMPREF];
    update();
    break;

  }

  /////////////////////////////
  // keys for walk sim mode
  if (currentMode == WALKSIMMODE){
    switch (event->key()) {
      case Qt::Key_Right:
        simulationStep();
        break;

      case Qt::Key_S:
        motion_sim_->setWalkRequest(false,0,0,0);
        update();
        break;
      case Qt::Key_I:
        motion_sim_->incrWalkRequest(true,0.1,0,0);
        update();
        break;
      case Qt::Key_J:
        motion_sim_->incrWalkRequest(true,0,0.1,0);
        update();
        break;
      case Qt::Key_L:
        motion_sim_->incrWalkRequest(true,0,-0.1,0);
        update();
        break;
      case Qt::Key_K:
        motion_sim_->incrWalkRequest(true,-0.1,0,0);
        update();
        break;
      case Qt::Key_U:
        motion_sim_->incrWalkRequest(true,0,0,0.1);
        update();
        break;
      case Qt::Key_O:
        motion_sim_->incrWalkRequest(true,0,0,-0.1);
        update();
        break;
      case Qt::Key_P:
        motion_sim_->walk_request_->walk_to_target_ = true;
        motion_sim_->walk_request_->rotate_around_target_ = false;
        motion_sim_->walk_request_->motion_ =WalkRequestBlock::WALK;
        update();
        break;

      case Qt::Key_R:
        motion_sim_->walk_request_->rotate_around_target_ = true;
        motion_sim_->walk_request_->walk_to_target_ = false;
        motion_sim_->walk_request_->rotate_distance_ = 250;
        motion_sim_->walk_request_->rotate_heading_ = -120.0*DEG_T_RAD;
        motion_sim_->walk_request_->motion_ =WalkRequestBlock::WALK;
        update();
        break;

      case Qt::Key_BracketLeft:
        motion_sim_->walk_request_->setKick(3000, 0, true, false);
        update();
        break;

      case Qt::Key_BracketRight:
        motion_sim_->walk_request_->setKick(3000, 0, false, false);
        update();
        break;
    }
  }

  ////////////////////////////
  // get up mode
  else if (currentMode == GETUPSIMMODE){
    // stuff
    switch (event->key()) {
    case Qt::Key_Right:
      simulationStep();
      break;
    case Qt::Key_B:
      motion_sim_->walk_request_->motion_=WalkRequestBlock::FALLING;
      motion_sim_->walk_request_->tilt_fallen_counter_ = -30;
      motion_sim_->getUpSide = -1;
      simulationStep();
      break;
    case Qt::Key_F:
      motion_sim_->walk_request_->motion_=WalkRequestBlock::FALLING;
      motion_sim_->walk_request_->tilt_fallen_counter_ = 30;
      motion_sim_->getUpSide = 1;
      simulationStep();
      break;

    }
  }

  ///////////////////////////
  // kick sim mode
  else if (currentMode == KICKSIMMODE){

    // stuff
    switch (event->key()) {
    case Qt::Key_Right:
      simulationStep();
      break;

      // change kick type
    case Qt::Key_0:
      motion_sim_->kick_request_->kick_type_ = Kick::NO_KICK;
      update();
      break;
    case Qt::Key_1:
      motion_sim_->kick_request_->kick_type_ = Kick::STRAIGHT;
      motion_sim_->kick_request_->kick_running_ = true;
      update();
      break;
    case Qt::Key_2:
      motion_sim_->kick_request_->kick_type_ = Kick::IKSTRAIGHT;
      motion_sim_->kick_request_->kick_running_ = true;
      update();
      break;
    case Qt::Key_3:
      motion_sim_->kick_request_->kick_type_ = Kick::SIDE;
      motion_sim_->kick_request_->kick_running_ = true;
      update();
      break;
    case Qt::Key_4:
      motion_sim_->kick_request_->kick_type_ = Kick::ANGLE;
      motion_sim_->kick_request_->kick_running_ = true;
      update();
      break;
    case Qt::Key_5:
      motion_sim_->kick_request_->kick_type_ = Kick::DEG30;
      motion_sim_->kick_request_->kick_running_ = true;
      update();
      break;

      // change kick leg
    case Qt::Key_L:
      motion_sim_->kick_request_->kick_leg_ = Kick::LEFT;
      update();
      break;
    case Qt::Key_R:
      if (event->modifiers() & Qt::ControlModifier) {
        motion_sim_->getLuaParameters();
        createKickSplines();
        update();
      } else {
        motion_sim_->kick_request_->kick_leg_ = Kick::RIGHT;
        update();
      }
      break;
    case Qt::Key_S:
      if (event->modifiers() & Qt::ControlModifier) {
        motion_sim_->vcore_->interpreter_->saveKickParameters(kick_params_->params_);
      } else {
        motion_sim_->kick_request_->kick_leg_ = Kick::SWITCHABLE;
        update();
      }
      break;

      // change desired distance / bearing
    case Qt::Key_Plus:
      motion_sim_->kick_request_->desired_distance_ += 500;
      update();
      break;
    case Qt::Key_BracketLeft:
      motion_sim_->kick_request_->desired_angle_ += DEG_T_RAD *5;
      update();
      break;
    case Qt::Key_BracketRight:
      motion_sim_->kick_request_->desired_angle_ -= DEG_T_RAD *5;
      update();
      break;
    case Qt::Key_Minus:
      motion_sim_->kick_request_->desired_distance_ -= 500;
      update();
      break;

      // change ball image center
    case Qt::Key_U:
      motion_sim_->kick_request_->ball_rel_x_ += 5;
      motion_sim_->kick_request_->ball_image_center_y_ += 5;
      update();
      break;
    case Qt::Key_H:
      motion_sim_->kick_request_->ball_rel_y_ += 5;
      motion_sim_->kick_request_->ball_image_center_x_ -= 5;
      update();
      break;
    case Qt::Key_K:
      motion_sim_->kick_request_->ball_rel_y_ -= 5;
      motion_sim_->kick_request_->ball_image_center_x_ += 5;
      update();
      break;
    case Qt::Key_J:
      motion_sim_->kick_request_->ball_rel_x_ -= 5;
      motion_sim_->kick_request_->ball_image_center_y_ -= 5;
      update();
      break;

    // move spline point
    case Qt::Key_Apostrophe:
      moveSplinePoint(-1,0,0);
      update();
      break;
    case Qt::Key_Comma:
      moveSplinePoint(1,0,0);
      update();
      break;
    case Qt::Key_A:
      moveSplinePoint(0,-1,0);
      update();
      break;
    case Qt::Key_O:
      moveSplinePoint(0,1,0);
      update();
      break;
    case Qt::Key_Semicolon:
      moveSplinePoint(0,0,-1);
      update();
      break;
    case Qt::Key_Q:
      moveSplinePoint(0,0,1);
      update();
      break;
    // add/remove points
    case Qt::Key_Delete:
      removeSplinePoint();
      update();
      break;
    case Qt::Key_Insert:
      addSplinePoint();
      update();
      break;

    // unselect spline point
    case Qt::Key_Escape:
      setSelectedName(-1);
      update();
      break;
    }
  }

  /////////////////////////////
  // normal modes
  else {
    switch (event->key()) {

    case Qt::Key_Left:
      emit prevSnapshot();
      break;
    case Qt::Key_Right:
      emit nextSnapshot();
      break;
    case Qt::Key_Up:
      emit play();
      break;
    case Qt::Key_Down:
      emit pause();
      break;
    }
  }
}


//////////////////////////////
// motion sim



void MotionGLWidget::initMotionSim(bool kick){
  std::cout << "initMotionSim" << std::endl << std::flush;
  if (motion_sim_ == NULL){
    motion_sim_ = new MotionSimulation(memory_);
  }

  motion_sim_->walk_request_->tilt_fallen_counter_ = 0;
  motion_sim_->walk_request_->roll_fallen_counter_ = 0;


  if (!kick)
    motion_sim_->setWalkRequest(false,1.0,0,0);
  if (kick)
    motion_sim_->walk_request_->noWalk();

  // do one step
  simulationStep();

  // clear graphable window
  //simLog->clear();
  //((UTMainWnd*)parent)->plotWnd_->setMemoryLog(simLog);
}

void MotionGLWidget::simulationStep(){
  motion_sim_->processFrame();

  // overwrite our memory with sim memory
  updateMemory(motion_sim_->memory_);
  update();

  // update joints window
  //((UTMainWnd*)parent)->jointsWnd_->update(motion_sim_->memory_);

  // update memory blocks in log window
  //((UTMainWnd*)parent)->logWnd_->updateMemoryBlocks(motion_sim_->memory_);

  // update graphable as well
  //simLog->push_back(*(motion_sim_->memory_));
  //((UTMainWnd*)parent)->plotWnd_->setMemoryLog(simLog);
  //((UTMainWnd*)parent)->plotWnd_->update(motion_sim_->memory_);
}


// helper functions
Pose3D MotionGLWidget::globalToDrawingFrame(Pose3D a){

  // convert from global to stance foot frame of reference
  a = a.globalToRelative(walk_engine_->global_frame_offset_);

  // now get stance foot
  Pose3D* stance_foot_;
  if (walk_engine_->step_current_.is_left_foot_){
    stance_foot_ = body_model_->getAbsPartPtr(BodyPart::left_bottom_foot);
  } else {
    stance_foot_ = body_model_->getAbsPartPtr(BodyPart::right_bottom_foot);
  }

  // convert from stance frame of ref to drawing frame of ref
  Pose2D stance_foot_2d_;
  stance_foot_2d_.translation.x = stance_foot_->translation.x;
  stance_foot_2d_.translation.y = stance_foot_->translation.y;
  stance_foot_2d_.rotation = stance_foot_->rotation.getZAngle();
  a = a.relativeToGlobal(*stance_foot_);

  return a;

}

Pose3D MotionGLWidget::globalToDrawingFrame(Pose2D a){
  Pose3D b(0,0,0);
  b.translation.x = a.translation.x;
  b.translation.y = a.translation.y;
  b.rotation.rotateZ(a.rotation);
  return globalToDrawingFrame(b);
}

Pose3D MotionGLWidget::globalToDrawingFrame(Vector2<float> a){
  Pose3D b(0,0,0);
  b.translation.x = a.x;
  b.translation.y = a.y;
  return globalToDrawingFrame(b);
}

void MotionGLWidget::drawSequence(const Keyframe& start, const Keyframe& finish, int cframe) {
  float progress = (cframe + 1.0f) / finish.frames;
  for(int i = 0; i < NUM_JOINTS; i++) {
    auto delta = (finish.joints[i] - start.joints[i]) * progress;
    lastKeyframe_.joints[i] = start.joints[i] + delta;
  }
  useKeyframes_ = true;
  update();
}

void MotionGLWidget::drawKeyframe(const Keyframe& keyframe) {
  lastKeyframe_ = keyframe;
  useKeyframes_ = true;
  update();
}
