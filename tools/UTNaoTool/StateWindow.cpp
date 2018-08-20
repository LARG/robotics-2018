#include <QtGui>
#include "StateWindow.h"
#include <iostream>
#include <memory/AudioProcessingBlock.h>

using namespace std;

StateWindow::StateWindow() : QWidget() {
  QGridLayout *layout = new QGridLayout;

  int i = 0;
  // from frame info
  names[i++] = "Frame #";
  names[i++] = "Time";
  names[i++] = "Memory Source";

  // from robot state
  names[i++] = "WO SELF";
  names[i++] = "Head Version";
  names[i++] = "Body Version";
  names[i++] = "Robot ID";
  names[i++] = "Body ID";
  names[i++] = "RB Team";
  names[i++] = "Role";

  // from game state
  names[i++] = "State";
  names[i++] = "GC Team";
  names[i++] = "Penalty Seconds";
  names[i++] = "Our Score";
  names[i++] = "Their Score";
  names[i++] = "Seconds Left";
  names[i++] = "PK State";
  names[i++] = "Whistle Time";

  // from audio processing
  names[i++] = "Processing Audio";
  names[i++] = "Whistle Frame";
  names[i++] = "Teammate Whistle Frame";

  labels = new QLabel[NumItems];
  values = new QLabel[NumItems];

  // set items
  for (int i = 0; i < NumItems; i++) {
    labels[i].setText(names[i]);
    values[i].setText(QString::number(0.0));

    // add to layout
    layout->addWidget(&labels[i], i, 0);
    layout->addWidget(&values[i], i, 1);
  }
  values[BodyID].setText("");
  setLayout(layout);
  
  resize(120,200);

  setWindowTitle(tr("Robot/Game State"));
}

void StateWindow::update(MemoryFrame* memory) {

  // get frame info and fill in
  FrameInfoBlock* frameInfo = NULL;
  memory->getBlockByName(frameInfo, "vision_frame_info",false);

  // fill it in if we've got it
  if (frameInfo != NULL){
    values[FrameNumber].setText(QString::number(frameInfo->frame_id,'f',0));
    values[Time].setText(QString::number(frameInfo->seconds_since_start,'f',3));
    if (frameInfo->source == MEMORY_ROBOT)
      values[MemorySource].setText("Robot");
    else
      values[MemorySource].setText("Sim");
  } 
  // or not available
  else {
    values[FrameNumber].setText("NA");
    values[Time].setText("NA");
    values[MemorySource].setText("NA");
  } 

  // get robot state and fill in
  RobotStateBlock* robotState = NULL;
  memory->getBlockByName(robotState, "robot_state", false);

  // fill it in if we've got it
  if (robotState != NULL){
    values[WO_SELF].setText(QString::number(robotState->WO_SELF,'f',0));
    values[RobotID].setText(QString::number(robotState->robot_id_,'f',0));
    values[HeadVersion].setText(QString::number(robotState->head_version_,'f',0));
    values[BodyVersion].setText(QString::number(robotState->body_version_,'f',0));
    values[BodyID].setText(robotState->bodyId().c_str());
    values[Role].setText(roleNames[robotState->role_].c_str());
    if (robotState->team_ == TEAM_RED)
      values[RBTeam].setText("RED");
    else
      values[RBTeam].setText("BLUE");
  }
  // or not available
  else {
    values[WO_SELF].setText("NA");
    values[RBTeam].setText("NA");
    values[Role].setText("NA");
    values[State].setText("NA");
  } 

  // get game state and fill in
  GameStateBlock* gameState = NULL;
  memory->getBlockByName(gameState, "game_state", false);

  // fill it in if we've got it
  if (gameState != NULL){
    values[State].setText(stateNames[gameState->state()].c_str());
    values[GCTeam].setText(QString::number(gameState->gameContTeamNum,'f',0));
    values[PenaltySeconds].setText(QString::number(gameState->secsTillUnpenalised,'f',0));
    values[OurScore].setText(QString::number(gameState->ourScore,'f',0));
    values[TheirScore].setText(QString::number(gameState->opponentScore,'f',0));
    values[SecondsLeft].setText(QString::number(gameState->secsRemaining,'f',0));
    values[PKState].setText(gameState->isPenaltyKick ? "PK" : "Normal");
    values[WhistleTime].setText(QString::number(gameState->whistleTime));
  }
  // or not available
  else {
    values[State].setText("N/A");
    values[GCTeam].setText("N/A");
    values[PenaltySeconds].setText("N/A");
    values[OurScore].setText("N/A");
    values[TheirScore].setText("N/A");
    values[SecondsLeft].setText("N/A");
    values[PKState].setText("N/A");
  }

  AudioProcessingBlock* audioProcessing = NULL;
  memory->getBlockByName(audioProcessing, "audio_processing", false);
  if (audioProcessing != NULL) {
    values[ProcessingAudio].setText(AudioProcessingBlock::getName(audioProcessing->state_));
    values[WhistleFrame].setText(QString::number(audioProcessing->whistle_heard_frame_));
    values[TeammateWhistleFrame].setText(QString::number(audioProcessing->teammate_heard_frame_));
  } else {
    values[ProcessingAudio].setText("NA");
    values[WhistleFrame].setText("NA");
  }

 
}
