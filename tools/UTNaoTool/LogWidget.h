#pragma once

#include <QtGui>
#include <QWidget>

#include "ui_LogWidget.h"
#include "simulation/LocalizationSimulation.h"

class WorldGLWidget;
class AnnotationGroup;
class LocalizationAnnotation;

class LAWidgetItem : public QListWidgetItem {
  public:
    int frame;
    Pose2D pose;
    LAWidgetItem(LocalizationAnnotation* la);
};

class LogWidget : public QWidget, public Ui_LogWidget {
  Q_OBJECT

  public:
    LogWidget(QWidget* parent);
    Qt::KeyboardModifiers modifiers();
    void setSimulation(Simulation* simulation);

  public slots:
    void frameLoaded(int frame);
    void fieldHovered(Point2D pos);
    void fieldClicked(Point2D pos, Qt::MouseButton button);
    void fieldDragged(Point2D start, Point2D end, Qt::MouseButton button);
    void setWorld(WorldGLWidget* world) { world_ = world; }
    void updateAnnotations(AnnotationGroup* annotations);
    void updatePoseCheckpoints();

  private:
    Simulation* simulation_;
    WorldGLWidget* world_;
    AnnotationGroup* annotations_;
    int frame_;
};
