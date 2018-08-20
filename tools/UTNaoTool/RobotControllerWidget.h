#ifndef ROBOT_CONTROLLER_WIDGET_H
#define ROBOT_CONTROLLER_WIDGET_H

#include <QtGui>
#include <qwt/qwt_dial_needle.h>
#include <Eigen/Core>
#include <set>
#include <iostream>

#include "RobotControllerConfig.h"
#include "ConfigWidget.h"
#include "ui_RobotControllerWidget.h"

#include <common/Poses.h>

class RobotControllerWidget : public ConfigWidget, public Ui_RobotControllerWidget { 
  Q_OBJECT
  public:
    RobotControllerWidget(QWidget* parent);

  protected:
    void keyPressEvent(QKeyEvent* kevent);
    void keyReleaseEvent(QKeyEvent* kevent);
    void focusInEvent(QFocusEvent* fevent);
    void focusOutEvent(QFocusEvent* fevent);
    Eigen::Vector3f processAccel(Eigen::Vector3f vel);
    void sendCommand();
    void loadConfig(const ToolConfig& config);
    void saveConfig(ToolConfig& config);

  protected slots:
    void loop();
    void stance();

  private:
    std::set<int> pressed_;
    QTimer* timer_;
    Eigen::Vector3f velocity_, command_;
    Poses::Stance stance_;
};

#endif
