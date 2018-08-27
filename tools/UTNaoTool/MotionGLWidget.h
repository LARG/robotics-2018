#ifndef MOTION_WIDGET_H
#define MOTION_WIDGET_H

#include <QGLViewer/qglviewer.h>
#include <QWidget>

#include <vector>

#include <tool/UTOpenGL/BasicGL.h>
#include <tool/UTOpenGL/RobotGL.h>

#include <memory/MemoryFrame.h>
#include <memory/LogViewer.h>
#include <memory/BodyModelBlock.h>
#include <memory/SensorBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/WalkEngineBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/WalkRequestBlock.h>
#include <memory/WalkParamBlock.h>
#include <memory/SensorCalibrationBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/KickRequestBlock.h>
#include <memory/KickEngineBlock.h>
#include <memory/KickModuleBlock.h>
#include <memory/KickParamBlock.h>


#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Spline3D.h>
#include <common/RobotDimensions.h>
#include <common/RobotInfo.h>
#include <common/RingQueue.h>
#include <common/Keyframe.h>

#include <common/MassCalibration.h>

#define TIME_TO_HEIGHT 200.0

class MotionSimulation;

class MotionGLWidget : public QGLViewer {
Q_OBJECT        // must include this if you use Qt signals/slots
 public:
  MotionGLWidget(QWidget* parent);

  enum {  // Display modes
    BODYMODELMODE,
    JOINTVALUESMODE,
    JOINTCOMMANDSMODE,
    WALKSIMMODE,
    WALKMODE,
    KICKMODE,
    KICKSIMMODE,
    GETUPSIMMODE,
    KEYFRAMEMODE,
    NUM_MODES
  };
  
  enum { // Display options
    SHOW_BODYMODEL,
    SHOW_JOINTVALUESMODEL,
    SHOW_JOINTCOMMANDSMODEL,
    SHOW_SENSORSTEXT,
    SHOW_WALKTEXT,
    SHOW_FRAMETEXT,
    SHOW_WALKREQUESTTEXT,
    SHOW_STEPS,
    SHOW_STEPSTEXT,
    SHOW_ZMPREF,
    SHOW_CURRENTZMP,
    SHOW_DESIREDZMP,
    SHOW_SENSEDZMP,
    SHOW_CURRENTPEN,
    SHOW_DESIREDPEN,
    SHOW_ZMPPENTEXT,
    SHOW_SWINGFOOT,
    SHOW_FEETTEXT,
    SHOW_ABSFEETTARGETS,
    SHOW_ODOMETRYTEXT,
    SHOW_TARGETPT,
    SHOW_KICKFOOTTARGET,
    SHOW_KICKREQUESTTEXT,
    SHOW_KICKTARGETTEXT,
    SHOW_KICKINFO,
    SHOW_KICKSPLINE,
    NUM_DISPLAY_OPTIONS
  };



  virtual void draw();
  virtual void init();

  QWidget* parent;
  void updateMemory(MemoryFrame* mem);

 private:

  BasicGL basicGL_;
  RobotGL robotGL_;
  
  Vector3<float> stick_figure_[BodyFrame::NUM_POINTS];

  BodyModelBlock* body_model_;
  SensorBlock* sensors_;
  FrameInfoBlock* frame_;
  WalkEngineBlock* walk_engine_;
  JointBlock* joint_values_;
  JointCommandBlock* joint_commands_;
  WalkRequestBlock* walk_request_;
  WalkParamBlock* walk_param_;
  SensorCalibrationBlock* sensor_calibration_;
  OdometryBlock* odometry_;
  KickRequestBlock* kick_request_;
  KickEngineBlock* kick_engine_;
  KickModuleBlock* kick_module_;
  KickParamBlock* kick_params_;

  RobotDimensions dimensions_;
  MassCalibration mass_calibration_;

  int currentMode;
  void setMode(int mode);
  void setAllDisplayOptions(bool val);
  bool displayOptions[NUM_DISPLAY_OPTIONS];

  void calculateStickFigure(BodyModelBlock* bm);
  void drawBodyModel(BodyModelBlock* bodyModel, RGB color);
  void drawBodyModelFromJointCommands();
  void drawBodyModelFromJointValues();
  void drawBodyModelFromKeyframe(Keyframe keyframe);
  BodyModelBlock* getBodyModelFromJoints(array<float,NUM_JOINTS> joints);
  BodyModelBlock* getBodyModelFromJoints(vector<float> joints);
  BodyModelBlock* getBodyModelFromJoints(float *joints);
  void drawSteps();
  void drawStep(WalkEngineBlock::Step s, bool draw_on_ground);
  void drawZmpTrajectory(RGB color, RingQueue<Vector2<float>, 100> zmp);
  void drawZmp(RGB color, Vector2<float> zmp, float time);
  void drawPen(RGB color, Vector2<float> pen, float time);
  void drawSwingFoot();
  void drawAbsFeetTargets();
  void drawTargetPoint();

  void drawFoot(Pose3D &foot, RGB &color);

  void drawKickFootTarget();
  void drawKickSpline(bool names = false);
  void addTimeToSwingSplinePoint(Vector3<float> &pt,float time_frac);

  Pose3D globalToDrawingFrame(Pose3D a);
  Pose3D globalToDrawingFrame(Pose2D a);
  Pose3D globalToDrawingFrame(Vector2<float> a);

  QString Vector2ToString(Vector2<float> val);
  QString Vector3ToString(Vector3<float> val);

  void textSensors();
  void textWalk();
  void textFrame();
  void textWalkRequest();
  void textSteps();
  void textZmpCom();
  void textFeet();
  void textOdometry();
  void textKickFeetTargets();
  void textKickRequest();
  void textKickInfo();

  // motion simulation
  MotionSimulation* motion_sim_;
  MemoryFrame* memory_;
  void initMotionSim(bool kick);
  void simulationStep();
  LogViewer* simLog;

  void createKickSplines();
  void calcKickSplinePoints();
  void moveSplinePoint(int x, int y, int z);
  void removeSplinePoint();
  void addSplinePoint();
  Spline3D kick_stance_spline_;
  Spline3D kick_swing_spline_;
  std::vector<Vector3<float> > kick_stance_spline_pts_;
  std::vector<Vector3<float> > kick_swing_spline_pts_;
  KickState::State prev_kick_state_;

  void drawWithNames();
  qglviewer::Vec orig, dir, selectedPoint;
  bool useKeyframes_;
  Keyframe lastKeyframe_;
  SupportBase base_;

 protected:
  void keyPressEvent(QKeyEvent *event);
  
 signals:
  void prevSnapshot();
  void nextSnapshot();
  void play();
  void pause();
  void modeChanged(QString);
public slots:
  void drawSequence(const Keyframe& start, const Keyframe& finish, int cframe);
  void drawKeyframe(const Keyframe& keyframe);
  void setSupportBase(SupportBase base) { base_ = base; update(); }
};

	
#endif
