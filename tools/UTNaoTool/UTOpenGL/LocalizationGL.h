#ifndef LOCALIZATION_GL_H
#define LOCALIZATION_GL_H

#include <QGLViewer/qglviewer.h>
#include "BasicGL.h"
#include "ObjectsGL.h"
#include "RobotGL.h"

#include <math/Pose3D.h>

#include <memory/WorldObjectBlock.h>
#include <memory/OdometryBlock.h>
#include <memory/RobotStateBlock.h>
#include <memory/LocalizationBlock.h>

class LocalizationGL {
public:
  void drawUncertaintyEllipse(Point2D loc, Point2D sd);
  void drawUncertaintyEllipse(Point2D loc, Eigen::Matrix2f cov);
  void drawUncertaintyEllipse(Pose2D pose, Eigen::Matrix2f cov);
  void drawUncertaintyAngle(Point2D loc, double ori, double sdOri);
  void drawUncertaintyAngle(Pose2D pose, double var);
  void drawRelativeObjects(WorldObjectBlock* gtObjects, WorldObjectBlock* beliefObjects, RobotStateBlock* robotState);
  void drawRelativeObjectUncerts(WorldObjectBlock* gtObjects, WorldObjectBlock* beliefObjects, RobotStateBlock* robotState, LocalizationBlock* localization);
  void drawObservationLine(Vector3<float> origin, Vector3<float> end, RGB color);
  void drawParticles(const std::vector<Particle>& particles);
 
  // Particle filter specific drawing  
  void drawOdometry(Point2D loc, AngRad ori, OdometryBlock* odometry);

  BasicGL basicGL;
  ObjectsGL objectsGL;
  RobotGL robotGL;
};


#endif


