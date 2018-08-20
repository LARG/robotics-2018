#ifndef SENSOR_WINDOW_H
#define SENSOR_WINDOW_H

#include <QWidget>

#include <memory/MemoryFrame.h>
#include <memory/SensorBlock.h>
#include <memory/SensorCalibrationBlock.h>
#include <common/RobotInfo.h>

class QLabel;
class QWidget;

class SensorWindow : public QWidget {
 Q_OBJECT

  public:
  SensorWindow();
    
  void update(MemoryFrame* memory);

  QLabel* sensorLabels;
  QLabel* rawLabels;
  QLabel* processedLabels;
  QLabel* visionLabels;
  QLabel* rawDeltaLabels;
  QLabel* processedDeltaLabels;
  QLabel* visionDeltaLabels;

  QLabel* sensorLeftSonarLabels;
  QLabel* rawLeftSonarLabels;
  QLabel* processedLeftSonarLabels;
  QLabel* visionLeftSonarLabels;

  QLabel* sensorRightSonarLabels;
  QLabel* rawRightSonarLabels;
  QLabel* processedRightSonarLabels;
  QLabel* visionRightSonarLabels;

  int numSonarValues;
};

#endif
