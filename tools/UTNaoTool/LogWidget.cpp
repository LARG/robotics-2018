#include "LogWidget.h"
#include "UTMainWnd.h"
#include "WorldGLWidget.h"
#include <common/annotations/AnnotationGroup.h>
#include <common/annotations/LocalizationAnnotation.h>

LAWidgetItem::LAWidgetItem(LocalizationAnnotation* la) {
  pose = la->pose();
  frame = la->frame();
  char buffer[256];
  snprintf(buffer, 256, "Frame %i: %2.f,%2.f @ %2.1f", frame, pose.translation.x, pose.translation.y, pose.rotation * RAD_T_DEG);
  setText(QString::fromUtf8(buffer));
}

LogWidget::LogWidget(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  simulation_ = NULL;
  annotations_ = NULL;
}

Qt::KeyboardModifiers LogWidget::modifiers() {
  return world_->modifiers();
}

void LogWidget::setSimulation(Simulation* simulation) {
  simulation_ = simulation;
}

void LogWidget::frameLoaded(int frame) {
  frame_ = frame;
}

void LogWidget::fieldHovered(Point2D pos) {
  mouseX->setText(QString::number(pos.x, 'f', 0));
  mouseY->setText(QString::number(pos.y, 'f', 0));
}

void LogWidget::fieldClicked(Point2D pos, Qt::MouseButton button) {
}

void LogWidget::fieldDragged(Point2D start, Point2D end, Qt::MouseButton button) {
  if(!annotations_) return;
  if(!UTMainWnd::inst()->isStreaming()) return;
  float orientation = (end - start).getDirection();
  Pose2D pose(orientation, start.x, start.y);
  auto la = new LocalizationAnnotation(pose, frame_);
  annotations_->addLocalizationAnnotation(la);
  updatePoseCheckpoints();
}

void LogWidget::updateAnnotations(AnnotationGroup* annotations) {
  annotations_ = annotations;
  updatePoseCheckpoints();
}

void LogWidget::updatePoseCheckpoints() {
  poses->clear();
  for(auto la : annotations_->getLocalizationAnnotations()) {
    auto wi = new LAWidgetItem(la);
    poses->addItem(wi);
  }
}
