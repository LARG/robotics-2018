#ifndef WORLD_GL_WIDGET_H
#define WORLD_GL_WIDGET_H

#include <QtGui>
#include <QTimer>
#include <QWidget>
#include <QGLViewer/qglviewer.h>

#include <set>

#include <VisionCore.h>
#include <memory/MemoryCache.h>

#include <math/Pose3D.h>
#include <math/Pose2D.h>
#include <math/Geometry.h>

#include <tool/UTOpenGL/GLDrawer.h>
#include <tool/UTOpenGL/BasicGL.h>

#include <tool/UTMainWnd.h>
#include <tool/LogWindow.h>
#include <tool/WalkWindow.h>
#include <tool/StateWindow.h>
#include <tool/SensorWindow.h>
#include <tool/JointsWindow.h>

class Simulation;
class AnnotationGroup;

class WorldGLWidget : public QGLViewer {
  Q_OBJECT        // must include this if you use Qt signals/slots
  public:
    WorldGLWidget(QWidget* parent);

    virtual void draw();
    void draw(Simulation* simulation);
    virtual void init();
    void mousePressEvent(QMouseEvent* mevent);
    void mouseReleaseEvent(QMouseEvent* mevent);
    void mouseMoveEvent(QMouseEvent* mevent);
    void keyPressEvent(QKeyEvent* kevent);
    void keyReleaseEvent(QKeyEvent* kevent);
    void focusOutEvent(QFocusEvent* fevent) { kpressed_.clear(); modifiers_ = Qt::NoModifier; }
    void processCameraKeys();


    enum { // Camera Positions
      OVERHEAD,
      OVERHEADREV,
      DEFENSIVEHALF,
      OFFENSIVEHALF,
      OFFENSIVEISO,
      DEFENSIVEISO,
      ABOVEROBOT,
      NUM_CAMS
    };

    void loadState(char* fileName);
    void saveState(char* fileName);
    void updateMemory(MemoryCache cache);
    void setCamera(int position);
    void setSimulation(Simulation* simulation);
    Qt::KeyboardModifiers modifiers() { return modifiers_; }

  private:
    MemoryCache cache_;
    GLDrawer drawer_;
    std::map<GLDrawer::DisplayOption, bool> display_;
    std::set<int> kpressed_, mdragged_;
    std::map<int,Point2D> mpressed_;
    Qt::KeyboardModifiers modifiers_;
    Simulation* simulation_;
    Point2D projectedMousePosition(int x, int y);
    BasicGL basicGL_;
    AnnotationGroup* annotations_;

    struct {
      bool dragging;
      Point2D start, end;
      Qt::MouseButton button;
    } dragInfo_;

  public slots:
    void updateDisplay(const std::map<GLDrawer::DisplayOption, bool>& display);
    void dragStart(Point2D start, Point2D end, Qt::MouseButton button);
    void dragEnd(Point2D start, Point2D end, Qt::MouseButton button);
    void updateAnnotations(AnnotationGroup* annotations);

  signals:
    void clicked(Point2D position, Qt::MouseButton button);
    void released(Point2D position, Qt::MouseButton button);
    void moved(Point2D position);
    void groundDragging(Point2D start, Point2D end, Qt::MouseButton button);
    void groundDragged(Point2D start, Point2D end, Qt::MouseButton button);
};


#endif
