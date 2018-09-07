#ifndef JOINTS_WINDOW_H
#define JOINTS_WINDOW_H

#include <QWidget>
#include <QCheckBox>
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

  QCheckBox* setStiffs;
  QCheckBox allStiffs;

  QString jointNames[NUM_JOINTS+3];

public slots: 
  void sendStiffness();
  void allStiffnessToggle(bool toggle);

};

#endif
