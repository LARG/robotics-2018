#include <QtGui>
#include "JointsWindow.h"
#include <iostream>
#include <tool/UTMainWnd.h>

using namespace std;

JointsWindow::JointsWindow() : QWidget() {
  QGridLayout *layout = new QGridLayout;

  jointNames[HeadYaw]="HeadYaw";
  jointNames[HeadPitch]="HeadPitch";
  jointNames[LShoulderPitch]="LShoulderPitch";
  jointNames[LShoulderRoll]="LShoulderRoll";
  jointNames[LElbowYaw]="LElbowYaw";
  jointNames[LElbowRoll]="LElbowRoll";
  jointNames[LHipYawPitch]="LHipYawPitch";
  jointNames[LHipPitch]="LHipPitch";
  jointNames[LHipRoll]="LHipRoll";
  jointNames[LKneePitch]="LKneePitch";
  jointNames[LAnklePitch]="LAnklePitch";
  jointNames[LAnkleRoll]="LAnkleRoll";
  jointNames[RHipYawPitch]="RHipYawPitch";
  jointNames[RHipPitch]="RHipPitch";
  jointNames[RHipRoll]="RHipRoll";
  jointNames[RKneePitch]="RKneePitch";
  jointNames[RAnklePitch]="RAnklePitch";
  jointNames[RAnkleRoll]="RAnkleRoll";
  jointNames[RShoulderPitch]="RShoulderPitch";
  jointNames[RShoulderRoll]="RShoulderRoll";
  jointNames[RElbowYaw]="RElbowYaw";
  jointNames[RElbowRoll]="RElbowRoll";

  jointNames[NUM_JOINTS]= "Time";
  jointNames[NUM_JOINTS+1]= "Send";
  jointNames[NUM_JOINTS+2]= "Mem";

  
  QLabel* jointLabel = new QLabel("Joint");
  QLabel* valueLabel = new QLabel("CurrAngle");
  QLabel* stiffLabel = new QLabel("CurrStiff");

  QLabel* commandAngleLabel = new QLabel("Cmd Angle");

  QLabel* commandStiffLabel = new QLabel("Cmd Stiff");
  QLabel* jointTempLabel = new QLabel("Temp");
  QLabel* jointChangeLabel = new QLabel("AngleChange");

  QLabel* setStiffnessLabel = new QLabel("SetStiffness");

  QLabel* allStiffnessLabel = new QLabel("All");

  jointLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  valueLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  stiffLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  commandAngleLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  commandStiffLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  jointTempLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  jointChangeLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  setStiffnessLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  allStiffnessLabel->setFont( QFont( "Arial", 10, QFont::Bold) );

  layout->addWidget(jointLabel,0,0);
  layout->addWidget(valueLabel,0,1);
  layout->addWidget(stiffLabel,0,2);
  layout->addWidget(commandAngleLabel,0,3);
  layout->addWidget(commandStiffLabel,0,4);
  layout->addWidget(jointTempLabel,0,5);
  layout->addWidget(jointChangeLabel,0,6);
  layout->addWidget(setStiffnessLabel,0,7);
  layout->addWidget(allStiffnessLabel, NUM_JOINTS+1, 7);

  jointLabels = new QLabel[NUM_JOINTS+3];
  jointValues = new QLabel[NUM_JOINTS+3];
  stiffValues = new QLabel[NUM_JOINTS+3];
  commandAngles = new QLabel[NUM_JOINTS+3];
  commandStiffs = new QLabel[NUM_JOINTS+3];
  jointTemps = new QLabel[NUM_JOINTS+3];
  jointChanges = new QLabel[NUM_JOINTS+3];
  setStiffs = new QCheckBox[NUM_JOINTS];

  // set joints
  for (int i = 0; i < NUM_JOINTS+3; i++) {
    jointLabels[i].setText(jointNames[i]);
    jointValues[i].setText("-");
    
    stiffValues[i].setText("-");

    commandAngles[i].setText("-");

    commandStiffs[i].setText("-");
    jointTemps[i].setText("-");
    jointChanges[i].setText("-");

    // add to layout
    layout->addWidget(&jointLabels[i], i+1, 0);
    layout->addWidget(&jointValues[i], i+1, 1);
    layout->addWidget(&stiffValues[i], i+1, 2);
    layout->addWidget(&commandAngles[i], i+1, 3);
    layout->addWidget(&commandStiffs[i], i+1, 4);
    layout->addWidget(&jointTemps[i], i+1, 5);
    layout->addWidget(&jointChanges[i], i+1, 6);
  }

  for (int i = 0; i < NUM_JOINTS; i++){
    layout->addWidget(&setStiffs[i], i+1, 7);
    connect(&setStiffs[i], SIGNAL(clicked()), this, SLOT(sendStiffness()));
  }
  layout->addWidget(&allStiffs, NUM_JOINTS+2, 7);
  connect(&allStiffs, SIGNAL(toggled(bool)), this, SLOT(allStiffnessToggle(bool)));
  setLayout(layout);

  resize(120,200);

  setWindowTitle(tr("Joints"));
}

// Send all the stiffness values
void JointsWindow::sendStiffness(){
  
  QString ip = UTMainWnd::inst()->getCurrentAddress();
  ToolPacket tp(ToolPacket::SetStiffness);

//  cout << "Going to send some data to " << ip.toStdString() << endl;
  for (int i = 0; i < NUM_JOINTS; i++){
    if (setStiffs[i].isChecked())
      tp.jointStiffness[i] = 1.0;
    else
      tp.jointStiffness[i] = 0.0;
    cout << tp.jointStiffness[i] << " ";
  }
//  cout << endl;

  UTMainWnd::inst()->sendUDPCommand(ip, tp);
  cout << "Joint stiffness sent" << endl;
}

void JointsWindow::allStiffnessToggle(bool toggle){
  for (int i = 0; i < NUM_JOINTS; i++){
    setStiffs[i].setChecked(toggle);
  }
  sendStiffness();
}

void JointsWindow::update(MemoryFrame* memory) {

  // Todd: try for motion first
  // then vision if we don't have motion

  // get joint angles
  JointBlock* joints = NULL;
  memory->getBlockByName(joints, "processed_joint_angles",false);
  jointValues[NUM_JOINTS+2].setText("PRO");
  stiffValues[NUM_JOINTS+2].setText("PRO");
  jointChanges[NUM_JOINTS+2].setText("PRO");
  if (joints == NULL){
    jointValues[NUM_JOINTS+2].setText("VIS");
    stiffValues[NUM_JOINTS+2].setText("VIS");
    jointChanges[NUM_JOINTS+2].setText("VIS");
    memory->getBlockByName(joints, "vision_joint_angles",false);
  }

  // if we got it, fill in values
  if (joints != NULL){
    for (int i = 0; i < NUM_JOINTS; i++){
      float degVal = joints->values_[i]*RAD_T_DEG;
      jointValues[i].setText(QString::number(degVal,'f',2));
      stiffValues[i].setText(QString::number(joints->stiffness_[i],'f',2));
      jointChanges[i].setText(QString::number(RAD_T_DEG*joints->getJointDelta(i),'f',2));
    }
  } 
  // otherwise, put NA
  else {
     for (int i = 0; i < NUM_JOINTS; i++){
       jointValues[i].setText("-");
       stiffValues[i].setText("-");
       jointChanges[i].setText("-");
     }
     jointValues[NUM_JOINTS+2].setText("NA");
     stiffValues[NUM_JOINTS+2].setText("NA");
     jointChanges[NUM_JOINTS+2].setText("NA");
  } 

  // other joint commands
  JointCommandBlock* jointCommands = NULL;
  memory->getBlockByName(jointCommands, "processed_joint_commands",false);
  commandAngles[NUM_JOINTS+2].setText("PRO");
  commandStiffs[NUM_JOINTS+2].setText("PRO");

  // try vision ones if we don't have motion ones
  if (jointCommands == NULL){
    commandAngles[NUM_JOINTS+2].setText("VIS");
    commandStiffs[NUM_JOINTS+2].setText("VIS");
    memory->getBlockByName(jointCommands, "vision_joint_commands",false);
  }
  if (jointCommands != NULL){
    for (int i = 0; i < NUM_JOINTS; i++){
      if (isnan(jointCommands->angles_[i])){
        commandAngles[i].setText("NAN");
      } else {
        commandAngles[i].setText(QString::number(jointCommands->angles_[i]*RAD_T_DEG,'f',2));
      }
      commandStiffs[i].setText(QString::number(jointCommands->stiffness_[i],'f',2));
    }
    commandAngles[NUM_JOINTS].setText(QString::number(jointCommands->body_angle_time_/1000.0,'f',2));
    if (jointCommands->send_body_angles_){
      commandAngles[NUM_JOINTS+1].setText("TRUE");
    } else {
      commandAngles[NUM_JOINTS+1].setText("FALSE");
    } 
    commandStiffs[NUM_JOINTS].setText(QString::number(jointCommands->stiffness_time_/1000.0,'f',2));
    if (jointCommands->send_stiffness_){
      commandStiffs[NUM_JOINTS+1].setText("TRUE");
    } else {
      commandStiffs[NUM_JOINTS+1].setText("FALSE");
    } 
  } else {
    for (int i = 2; i < NUM_JOINTS; i++){
      commandAngles[i].setText("-");
      commandStiffs[i].setText("-");
    }
    commandAngles[NUM_JOINTS+2].setText("NA");
    commandStiffs[NUM_JOINTS+2].setText("NA");
  }

  // get temperatures
  SensorBlock* sensors = NULL;
  memory->getBlockByName(sensors, "processed_sensors",false);
  jointTemps[NUM_JOINTS+2].setText("PRO");
  // try vision ones if we don't have motion ones
  if (sensors == NULL){
    jointTemps[NUM_JOINTS+2].setText("VIS");
    memory->getBlockByName(sensors, "vision_sensors",false);
  }
  if (sensors != NULL){
    for (int i = 0; i < NUM_JOINTS; i++){
      if (isnan(sensors->joint_temperatures_[i])){
        jointTemps[i].setText("NAN");
      } else {
        jointTemps[i].setText(QString::number(sensors->joint_temperatures_[i],'f',2));
      }
      
    }
  } else {
    for (int i = 0; i < NUM_JOINTS; i++){
      jointTemps[i].setText("-");
    }
    jointTemps[NUM_JOINTS+2].setText("NA");
  }

 
}
