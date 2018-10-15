#include "SimControlWidget.h"
#include "WorldGLWidget.h"
#include "simulation/LocalizationSimulation.h"
#include "simulation/BehaviorSimulation.h"

SimControlWidget::SimControlWidget(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  simulation_ = NULL;
  connect(btnFlip, SIGNAL(clicked()), this, SLOT(flip()));
  connect(btnPenalize, SIGNAL(clicked()), this, SLOT(penalize()));
  connect(btnHideBall, SIGNAL(clicked()), this, SLOT(hideBall()));
}

Qt::KeyboardModifiers SimControlWidget::modifiers() {
  return world_->modifiers();
}

void SimControlWidget::setSimulation(Simulation* simulation) {
  simulation_ = simulation;
  if(!simulation_) return;
  auto players = simulation->activePlayers();
  for(auto p : players) {
    stringstream ss;
    ss << (p <= WO_TEAM_LAST ? "Blue " : "Red ");
    int pnum = (p - WO_TEAM1 + 1) % (NUM_PLAYERS);
    if(!pnum) pnum = NUM_PLAYERS;
    ss << pnum;
    playerBox->addItem(ss.str().c_str(), QVariant(p));
  }
  if(playerBox->count()) playerBox->setCurrentIndex(0);
}

void SimControlWidget::penalize() {
  if(!simulation_) return;
  if(auto sim = dynamic_cast<BehaviorSimulation*>(simulation_)) {
    int player = playerBox->itemData(playerBox->currentIndex()).toInt();
    sim->setPenalty(player);
  }
}

void SimControlWidget::flip() {
  if(!simulation_) return;
  if(auto sim = dynamic_cast<LocalizationSimulation*>(simulation_)) {
    sim->flip();
  }
}

void SimControlWidget::fieldHovered(Point2D pos) {
  mouseX->setText(QString::number(pos.x, 'f', 0));
  mouseY->setText(QString::number(pos.y, 'f', 0));
}

void SimControlWidget::fieldClicked(Point2D pos, Qt::MouseButton button) {
  if(!simulation_) return;
  if(button == Qt::LeftButton && (modifiers() == Qt::ControlModifier)) {
    printf("move ball to %2.f,%2.f\n", pos.x, pos.y);
    simulation_->moveBall(pos);
  } else if (button == Qt::RightButton && (modifiers() == Qt::ControlModifier)) {
    printf("teleport robot to %2.f,%2.f\n", pos.x, pos.y);
    simulation_->teleportPlayer(pos, 0., 0);
  }
}

void SimControlWidget::fieldDragged(Point2D start, Point2D end, Qt::MouseButton button) {
  if(!playerBox->count()) return;
  if(!simulation_) return;
  if(modifiers() == (Qt::AltModifier | Qt::ControlModifier)) {
    float orientation = (end - start).getDirection();
    int player = playerBox->itemData(playerBox->currentIndex()).toInt();
    if(button == Qt::LeftButton)
      simulation_->movePlayer(start, orientation, player);
    else
      simulation_->teleportPlayer(start, orientation, player);
  }
}

void SimControlWidget::hideBall() {
  simulation_->moveBall(Point2D(10'000,10'000));
}
