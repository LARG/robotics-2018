#pragma once
#include <QtGui>
#include <QWidget>

#include "ui_SimControlWidget.h"
#include <math/Geometry.h>

class WorldGLWidget;
class Simulation;

class SimControlWidget : public QWidget, public Ui_SimControlWidget {
  Q_OBJECT

  public:
    SimControlWidget(QWidget* parent);
    Qt::KeyboardModifiers modifiers();
    void setSimulation(Simulation* simulation);

  public slots:
    void flip();
    void penalize();
    void fieldHovered(Point2D pos);
    void fieldClicked(Point2D pos, Qt::MouseButton button);
    void fieldDragged(Point2D start, Point2D end, Qt::MouseButton button);
    void setWorld(WorldGLWidget* world) { world_ = world; }

  private:
    Simulation* simulation_;
    WorldGLWidget* world_;
};
