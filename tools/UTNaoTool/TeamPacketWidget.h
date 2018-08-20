#ifndef TEAM_PACKET_WIDGET_H
#define TEAM_PACKET_WIDGET_H

#include <QtGui>

#include <common/Enum.h>
#include <memory/MemoryCache.h>
#include "ui_TeamPacketWidget.h"

class TeamPacketWidget : public QWidget, public Ui_TeamPacketWidget {
  Q_OBJECT
  public:
    TeamPacketWidget(QWidget* parent);
    void updateMemory(MemoryCache cache);

  protected:
    ENUM(RobotSelection,
      AllRobots,
      Robot1,
      Robot2,
      Robot3,
      Robot4,
      Robot5
    );

    void updateStatuses();
  protected slots:
    void setSelectedRobot(int i);
    void displayPacketData();
  private:
    MemoryCache cache_;
    std::map<RobotSelection,QLabel*> statuses_;
    std::map<RobotSelection,std::string> displays_;
    RobotSelection current_;
};

#endif
