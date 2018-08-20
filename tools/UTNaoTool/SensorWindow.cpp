#include <QtGui>
#include "SensorWindow.h"
#include <iostream>


using namespace std;

SensorWindow::SensorWindow() : QWidget() {
  QGridLayout *layout = new QGridLayout;

  QLabel* sensorLabel = new QLabel("Sensor");
  sensorLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  QLabel* rawLabel = new QLabel("Raw");
  rawLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  QLabel* rawDeltaLabel = new QLabel(QString::fromWCharArray(L"Raw Δ"));
  rawDeltaLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  QLabel* processedLabel = new QLabel("Proc");
  processedLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  QLabel* processedDeltaLabel = new QLabel(QString::fromWCharArray(L"Proc Δ"));
  processedDeltaLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );

  QLabel* visionLabel = new QLabel("Vis");
  visionLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  QLabel* visionDeltaLabel = new QLabel(QString::fromWCharArray(L"Vis Δ"));
  visionDeltaLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  
  layout->addWidget(sensorLabel,0,0);
  layout->addWidget(rawLabel,0,1);
  layout->addWidget(rawDeltaLabel,0,2);
  layout->addWidget(processedLabel,0,3);
  layout->addWidget(processedDeltaLabel,0,4);
  layout->addWidget(visionLabel,0,5);
  layout->addWidget(visionDeltaLabel,0,6);
  
  sensorLabels = new QLabel[NUM_SENSORS];
  rawLabels = new QLabel[NUM_SENSORS];
  processedLabels = new QLabel[NUM_SENSORS];
  visionLabels = new QLabel[NUM_SENSORS];
  rawDeltaLabels = new QLabel[NUM_SENSORS];
  processedDeltaLabels = new QLabel[NUM_SENSORS];
  visionDeltaLabels = new QLabel[NUM_SENSORS];

  numSonarValues = 1;
  if (numSonarValues > NUM_SONAR_VALS) 
    numSonarValues = NUM_SONAR_VALS;

  sensorLeftSonarLabels = new QLabel[numSonarValues];
  sensorRightSonarLabels = new QLabel[numSonarValues];
  rawLeftSonarLabels = new QLabel[numSonarValues];
  rawRightSonarLabels = new QLabel[numSonarValues];
  processedLeftSonarLabels = new QLabel[numSonarValues];
  processedRightSonarLabels = new QLabel[numSonarValues];
  visionLeftSonarLabels = new QLabel[numSonarValues];
  visionRightSonarLabels = new QLabel[numSonarValues];

  float maxWidth = 50.0f;
  for(int i = 0; i < NUM_SENSORS; i++) {
    rawLabels[i].setMaximumWidth(maxWidth);
    processedLabels[i].setMaximumWidth(maxWidth);
    visionLabels[i].setMaximumWidth(maxWidth);

    rawDeltaLabels[i].setMaximumWidth(maxWidth);
    processedDeltaLabels[i].setMaximumWidth(maxWidth);
    visionDeltaLabels[i].setMaximumWidth(maxWidth);
    
    rawLabels[i].setMinimumWidth(maxWidth);
    processedLabels[i].setMinimumWidth(maxWidth);
    visionLabels[i].setMinimumWidth(maxWidth);

    rawDeltaLabels[i].setMinimumWidth(maxWidth);
    processedDeltaLabels[i].setMinimumWidth(maxWidth);
    visionDeltaLabels[i].setMinimumWidth(maxWidth);
  }

  for(int i = 0; i < numSonarValues; i++) {
    sensorLeftSonarLabels->setMaximumWidth(maxWidth); 
    sensorRightSonarLabels->setMaximumWidth(maxWidth); 
    rawLeftSonarLabels->setMaximumWidth(maxWidth); 
    rawRightSonarLabels->setMaximumWidth(maxWidth); 
    processedLeftSonarLabels->setMaximumWidth(maxWidth); 
    processedRightSonarLabels->setMaximumWidth(maxWidth); 
    visionLeftSonarLabels->setMaximumWidth(maxWidth); 
    visionRightSonarLabels->setMaximumWidth(maxWidth); 
  }

  int offset = 0;

  // set joints
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorLabels[i].setText(getSensorString((Sensor)i));
    rawLabels[i].setText(QString::number(0));
    processedLabels[i].setText(QString::number(0));
    visionLabels[i].setText(QString::number(0));

    // add to layout
    layout->addWidget(&sensorLabels[i], offset+i+1, 0);
    layout->addWidget(&rawLabels[i], offset+i+1, 1);
    layout->addWidget(&processedLabels[i], offset+i+1, 3);
    layout->addWidget(&visionLabels[i], offset+i+1, 5);
    
    if(i >= FIRST_ANGLE_SENSOR && i <= LAST_ANGLE_SENSOR) {
      rawDeltaLabels[i].setText(QString::number(0));
      processedDeltaLabels[i].setText(QString::number(0));
      visionDeltaLabels[i].setText(QString::number(0));
      
      layout->addWidget(&rawDeltaLabels[i], offset+i+1, 2);
      layout->addWidget(&processedDeltaLabels[i], offset+i+1, 4);
      layout->addWidget(&visionDeltaLabels[i], offset+i+1, 6);
    }
  }
  offset += NUM_SENSORS;

  // raw sonar values
  for (int i = 0; i < numSonarValues; i++) {
    sensorLeftSonarLabels[i].setText(QString("US/Left") + QString::number(i));
    rawLeftSonarLabels[i].setText(QString::number(0));
    processedLeftSonarLabels[i].setText(QString::number(0));
    visionLeftSonarLabels[i].setText(QString::number(0));
    // add to layout
    layout->addWidget(&sensorLeftSonarLabels[i], offset+i+1, 0);
    layout->addWidget(&rawLeftSonarLabels[i], offset+i+1, 1);
    layout->addWidget(&processedLeftSonarLabels[i], offset+i+1, 3);
    layout->addWidget(&visionLeftSonarLabels[i], offset+i+1, 5);
  }
  offset += numSonarValues;

  // raw sonar values - right 
  for (int i = 0; i < numSonarValues; i++) {
    sensorRightSonarLabels[i].setText(QString("US/Right") + QString::number(i));
    rawRightSonarLabels[i].setText(QString::number(0));
    processedRightSonarLabels[i].setText(QString::number(0));
    visionRightSonarLabels[i].setText(QString::number(0));
    // add to layout
    layout->addWidget(&sensorRightSonarLabels[i], offset+i+1, 0);
    layout->addWidget(&rawRightSonarLabels[i], offset+i+1, 1);
    layout->addWidget(&processedRightSonarLabels[i], offset+i+1, 3);
    layout->addWidget(&visionRightSonarLabels[i], offset+i+1, 5);
  }
  offset += numSonarValues;
  setLayout(layout);

  resize(120,200);

  setWindowTitle(tr("Sensors"));
}

void SensorWindow::update(MemoryFrame* memory) {
  SensorBlock* raw_sensors = NULL;
  memory->getBlockByName(raw_sensors, "raw_sensors",false);
  SensorBlock* processed_sensors = NULL;
  memory->getBlockByName(processed_sensors, "processed_sensors",false);
  SensorBlock* vision_sensors = NULL;
  memory->getBlockByName(vision_sensors, "vision_sensors",false);
 
  if (raw_sensors!=NULL) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      float factor = 1.0;
      if(i >= FIRST_ANGLE_SENSOR && i <= LAST_ANGLE_SENSOR) {
        rawDeltaLabels[i].setText(QString::number(raw_sensors->getDelta(i) * RAD_T_DEG,'f',2));
        factor = RAD_T_DEG;
      }
      rawLabels[i].setText(QString::number(raw_sensors->values_[i] * factor,'f',2));
    }
    for (int i = 0; i < numSonarValues; i++) {
      rawLeftSonarLabels[i].setText(QString::number(raw_sensors->sonar_left_[i],'f',2));
      rawRightSonarLabels[i].setText(QString::number(raw_sensors->sonar_right_[i],'f',2));

    }
  }
  if (processed_sensors!=NULL) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      float factor = 1.0;
      if(i >= FIRST_ANGLE_SENSOR && i <= LAST_ANGLE_SENSOR) {
        processedDeltaLabels[i].setText(QString::number(processed_sensors->getDelta(i) * RAD_T_DEG,'f',2));
        factor = RAD_T_DEG;
      }
      processedLabels[i].setText(QString::number(processed_sensors->values_[i] * factor,'f',2));
    }
    for (int i = 0; i < numSonarValues; i++) {
      processedLeftSonarLabels[i].setText(QString::number(processed_sensors->sonar_left_[i],'f',2));
      processedRightSonarLabels[i].setText(QString::number(processed_sensors->sonar_right_[i],'f',2));

    }
  }
  if (vision_sensors!=NULL) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      float factor = 1.0;
      if(i >= FIRST_ANGLE_SENSOR && i <= LAST_ANGLE_SENSOR) {
        visionDeltaLabels[i].setText(QString::number(vision_sensors->getDelta(i) * RAD_T_DEG,'f',2));
        factor = RAD_T_DEG;
      }
      visionLabels[i].setText(QString::number(vision_sensors->values_[i] * factor,'f',2));
    }
    for (int i = 0; i < numSonarValues; i++) {
      visionLeftSonarLabels[i].setText(QString::number(vision_sensors->sonar_left_[i],'f',2));
      visionRightSonarLabels[i].setText(QString::number(vision_sensors->sonar_right_[i],'f',2));

    }
  }
}
