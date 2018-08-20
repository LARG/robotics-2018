#ifndef JOINTS_WINDOW_H
#define JOINTS_WINDOW_H

#include <QWidget>

#include <memory/MemoryFrame.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SensorBlock.h>
#include <common/RobotInfo.h>


class QLabel;
class QWidget;

class JointsWindow : public QWidget {
 Q_OBJECT

  public:
  JointsWindow();
    
  void update(MemoryFrame* memory);
  QLabel* jointLabels;
  QLabel* jointValues;
  QLabel* stiffValues;

  QLabel* commandAngles;
  QLabel* commandStiffs;
  QLabel* jointTemps;
  QLabel* jointChanges;
  
  QString jointNames[NUM_JOINTS+3];

  
};

#endif
