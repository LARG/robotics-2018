#include "RobotControllerWidget.h"
#include "UTMainWnd.h"
#include <common/ToolPacket.h>
#include "joystickxbox.h"

#define CROP(vec,MIN,MAX) for(int i = 0; i < vec.size(); i++) { vec[i] = std::max((float)MIN, std::min((float)MAX, (float)vec[i])); }
#define SNAP(vec,thresh) for(int i = 0; i < vec.size(); i++) { vec[i] = fabs(vec[i]) < thresh ? 0 : vec[i]; }

#define INTERVAL (1.0/30)

using namespace Eigen;

RobotControllerWidget::RobotControllerWidget(QWidget* parent) : ConfigWidget(parent) {
  setupUi(this);
  velCompass->setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Ray, false, Qt::darkGray, Qt::black));
  velCompass->setValue(30.0);
  timer_ = new QTimer(this);
  timer_->setInterval(INTERVAL * 1000);
  timer_->setSingleShot(false);
  timer_->start();
  connect(timer_, SIGNAL(timeout()), this, SLOT(loop()));
  connect(stanceButton, SIGNAL(clicked()), this, SLOT(stance()));
  connect(accelX, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  connect(accelY, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  connect(accelTheta, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  connect(maxVelX, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  connect(maxVelY, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  connect(maxVelTheta, SIGNAL(valueChanged(double)), UTMainWnd::inst(), SLOT(saveConfig(double)));
  velocity_ << 0,0,0;
  stance_ = Poses::SITTING;

  // Xbox control of robot
  initXboxJoystick();
}

void RobotControllerWidget::loop() {
  if(holdVelocities->isChecked()) return;
  Vector3f accel = processAccel(velocity_);
  velocity_ += accel * INTERVAL;
  CROP(velocity_, -1.0, 1.0);
  command_ = velocity_;
  SNAP(command_, .1);
  float direction = atan2f(command_[0], command_[1]) * 180 / M_PI - 90;
  if(command_[0] == 0 && command_[1] == 0)
    direction = 0;
  velCompass->setValue(direction);
  velSlider->setValue(command_[2] * -100);
  velX->setValue(command_[0]);
  velY->setValue(command_[1]);
  velTheta->setValue(command_[2]);
  static int i = 0; i++;
  if(i % 5 == 0)
    sendCommand();
}

void RobotControllerWidget::sendCommand() {
  if(!sendCommands->isChecked()) return;
  ToolPacket tp(ToolPacket::ManualControl);
  tp.odom_command.x = command_[0];
  tp.odom_command.y = command_[1];
  tp.odom_command.theta = command_[2];
  tp.odom_command.stance = stance_;
  UTMainWnd::inst()->sendUDPCommandToCurrent(tp);
}

void RobotControllerWidget::keyPressEvent(QKeyEvent* kevent) {
  pressed_.insert(kevent->key());
}

void RobotControllerWidget::keyReleaseEvent(QKeyEvent* kevent) {
  pressed_.erase(kevent->key());
}

void RobotControllerWidget::focusInEvent(QFocusEvent* fevent) {
  velCompass->setStyleSheet("background-color: black");
}

void RobotControllerWidget::focusOutEvent(QFocusEvent* fevent) {
  velCompass->setStyleSheet("");
  pressed_.clear();
}

Vector3f RobotControllerWidget::processAccel(Vector3f vel) {
  auto ispressed = [&] (int key) {
    return pressed_.find(key) != pressed_.end();
  };
  Vector3f accel;
  if(ispressed(Qt::Key_W)) 
    accel[0] = accelX->value();
  else if(ispressed(Qt::Key_S))
    accel[0] = -accelX->value();
  else
    accel[0] = -vel[0];
    
  if(ispressed(Qt::Key_Q)) 
    accel[1] = accelY->value();
  else if(ispressed(Qt::Key_E))
    accel[1] = -accelY->value();
  else
    accel[1] = -vel[1];
  
  if(ispressed(Qt::Key_A)) 
    accel[2] = accelTheta->value();
  else if(ispressed(Qt::Key_D))
    accel[2] = -accelTheta->value();
  else
    accel[2] = -vel[2];

  // XBox control
  if(updateXboxJoystick()) {
    ControllerInfo controllerData = getControllerInfo();
    if (abs(controllerData.x) >= .1 || abs(controllerData.y) >= .1) {
    accel[0] = controllerData.y;
    accel[1] = -1 * controllerData.x;

    } else {
    accel[0] = -vel[0];
    accel[1] = -vel[1];
      }


      if (abs(controllerData.y2) >= .05) {
    accel[2] = controllerData.y2;
      } else {
    accel[2] = -vel[2];
      }

      //if (controllerData.x2 > 0) {
        //player->_action = JS_PASS_ACTION;
      //}

      //if (controllerData.lt > .5) {
        //player->_action = JS_SHOOT_ACTION;
      //}
  }
  return accel;
}

void RobotControllerWidget::stance() {
  stance_ = stance_ == Poses::STANDING ? Poses::SITTING : Poses::STANDING;
  stanceButton->setText(stance_ == Poses::STANDING ? "Set to Sitting" : "Set to Standing");
}

void RobotControllerWidget::loadConfig(const ToolConfig& config) {
  auto cfg = config.rcConfig;
  accelX->setValue(cfg.accelX);
  accelY->setValue(cfg.accelY);
  accelTheta->setValue(cfg.accelTheta);
  maxVelX->setValue(cfg.maxVelX);
  maxVelY->setValue(cfg.maxVelY);
  maxVelTheta->setValue(cfg.maxVelTheta);
}

void RobotControllerWidget::saveConfig(ToolConfig& config) {
  auto& cfg = config.rcConfig;
  cfg.accelX = accelX->value();
  cfg.accelY = accelY->value();
  cfg.accelTheta = accelTheta->value();
  cfg.maxVelX = maxVelX->value();
  cfg.maxVelY = maxVelY->value();
  cfg.maxVelTheta = maxVelTheta->value();
}
