#include <QtGui>
#include "WalkWindow.h"
#include <iostream>


using namespace std;

WalkWindow::WalkWindow() : QWidget() {
  QGridLayout *layout = new QGridLayout;

  QLabel* cmdLabel = new QLabel("Command");
  QLabel* odomLabel = new QLabel("Odometry");
  QLabel* posLabel = new QLabel("Position");
  QLabel* velLabel = new QLabel("Velocity");
  QLabel* nextPosLabel = new QLabel("Next Pos");

  cmdLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  odomLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  posLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  velLabel->setFont( QFont( "Arial", 10, QFont::Bold) );
  nextPosLabel->setFont( QFont( "Arial", 10, QFont::Bold) );

  layout->addWidget(cmdLabel,0,1);
  layout->addWidget(odomLabel,0,2);
  layout->addWidget(posLabel,0,3);
  layout->addWidget(velLabel,0,4);
  layout->addWidget(nextPosLabel,0,5);

  walkLabels = new QLabel[4];
  walkCommands = new QLabel[4];
  walkOdometry = new QLabel[4];
  walkPosition = new QLabel[4];
  walkVelocity = new QLabel[4];
  walkNextPosition = new QLabel[4];

  walkLabels[0].setText("Fwd");
  walkLabels[1].setText("Left");
  walkLabels[2].setText("CCW");
  walkLabels[3].setText("Cmd");

  // set things
  for (int i = 0; i < 4; i++) {
    walkCommands[i].setText("-");
    walkOdometry[i].setText("-");
    walkPosition[i].setText("-");
    walkVelocity[i].setText("-");
    walkNextPosition[i].setText("-");

    // add to layout
    layout->addWidget(&walkLabels[i], i+1, 0);
    layout->addWidget(&walkCommands[i], i+1, 1);
    layout->addWidget(&walkOdometry[i], i+1, 2);
    layout->addWidget(&walkPosition[i], i+1, 3);
    layout->addWidget(&walkVelocity[i], i+1, 4);
    layout->addWidget(&walkNextPosition[i], i+1, 5);
  }
  setLayout(layout);

  resize(120,200);

  setWindowTitle(tr("Walk"));
}

void WalkWindow::update(MemoryFrame* memory) {

  // get walk request
  WalkRequestBlock* walkReq = NULL;
  memory->getBlockByName(walkReq, "vision_walk_request",false);
  if (walkReq != NULL){
    walkCommands[0].setText(QString::number(walkReq->speed_.translation.x,'f',2));
    walkCommands[1].setText(QString::number(walkReq->speed_.translation.y,'f',2));
    walkCommands[2].setText(QString::number(walkReq->speed_.rotation,'f',2));
    walkCommands[3].setText(QString(WalkRequestBlock::getName(walkReq->motion_)));
    
  } else {
    for (int i = 0; i < 4; i++){
      walkCommands[i].setText("NA");
    }
  }

  // get walk odometry
  OdometryBlock* odometry = NULL;
  memory->getBlockByName(odometry, "vision_odometry",false);
  if (odometry != NULL){
    walkOdometry[0].setText(QString::number(odometry->displacement.translation.x,'f',2));
    walkOdometry[1].setText(QString::number(odometry->displacement.translation.y,'f',2));
    walkOdometry[2].setText(QString::number(RAD_T_DEG*odometry->displacement.rotation,'f',2));
    if (odometry->getting_up_side_ == Getup::FRONT){
      walkOdometry[3].setText("GETUP FRONT");
    } else if (odometry->getting_up_side_ == Getup::BACK){
      walkOdometry[3].setText("GETUP BACK");
    } else if (odometry->getting_up_side_ == Getup::UNKNOWN){
      walkOdometry[3].setText("GETUP CROSS");
    } else if (odometry->standing){
      walkOdometry[3].setText("STAND");
    } else {
      walkOdometry[3].setText("WALK");
    }
    if (odometry->fall_direction_ == Fall::FORWARD){
      walkPosition[3].setText("FALL FWD");
    } else if (odometry->fall_direction_ == Fall::BACKWARD){
      walkPosition[3].setText("FALL BACK");
    } else if (odometry->fall_direction_ == Fall::LEFT){
      walkPosition[3].setText("FALL LEFT");
    } else if (odometry->fall_direction_ == Fall::RIGHT){
      walkPosition[3].setText("FALL RIGHT");
    } else {
      walkPosition[3].setText("NO FALL");
    } 
  } else {
    for (int i = 0; i < 4; i++){
      walkOdometry[i].setText("NA");
    }
  }

  // get walk info
  WalkInfoBlock* walkInfo = NULL;
  memory->getBlockByName(walkInfo, "vision_walk_info",false);
  if (walkInfo != NULL){
    walkPosition[0].setText(QString::number(walkInfo->robot_position_.translation.x,'f',2));
    walkPosition[1].setText(QString::number(walkInfo->robot_position_.translation.y,'f',2));
    walkPosition[2].setText(QString::number(RAD_T_DEG*walkInfo->robot_position_.rotation,'f',2));

    walkVelocity[0].setText(QString::number(walkInfo->robot_velocity_.translation.x,'f',2));
    walkVelocity[1].setText(QString::number(walkInfo->robot_velocity_.translation.y,'f',2));
    walkVelocity[2].setText(QString::number(RAD_T_DEG*walkInfo->robot_velocity_.rotation,'f',2));

    walkNextPosition[0].setText(QString::number(walkInfo->robot_relative_next_position_.translation.x,'f',2));
    walkNextPosition[1].setText(QString::number(walkInfo->robot_relative_next_position_.translation.y,'f',2));
    walkNextPosition[2].setText(QString::number(RAD_T_DEG*walkInfo->robot_relative_next_position_.rotation,'f',2));
  } else {
    for (int i = 0; i < 3; i++){
      walkPosition[i].setText("NA");
      walkVelocity[i].setText("NA");
      walkNextPosition[i].setText("NA");
    }
  }
}
