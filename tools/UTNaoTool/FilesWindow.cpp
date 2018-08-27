#include <QtGui>
#include "FilesWindow.h"
#include <iostream>
#include "UTMainWnd.h"
#include <stdio.h>
#include <tool/LogSelectWindow.h>

using namespace std;

FilesWindow::FilesWindow(QMainWindow* p) : ConfigWindow(p) {
  setupUi(this);
  setWindowTitle(tr("Files Window"));
  basePath = QString(getenv("NAO_HOME")) + "/";
  logPath = basePath + "logs/";
  dataPath = basePath + "data/";

  environment = QProcessEnvironment::systemEnvironment();
  environment.remove("PYTHONHOME");
  environment.remove("PYTHONPATH");
  isRobotTimeSet = false;
  // 1 is our team number for RC 2013
  teamNumBox->setValue(1);

  prevTime=prevTime.currentDateTime();

  parent = p;
  QPalette rspalette;
  robotStatus->setText("Unknown");
  rspalette.setColor(QPalette::WindowText, Qt::darkYellow);
  robotStatus->setPalette(rspalette);

  enableButtons(true);

  // start timer for status updates
  statusTimer = new QTimer(this);
  // check on robots using fping every 10 seconds (now variable based on whether ping succeeded)
	statusTimer->singleShot(10000,this,SLOT(checkStatus()));

  // until we get simulator back
  //locationBox->addItem("localhost"); // JM 2/6/2014 Why is this necessary?
  locationBox->addItem("core");
  std::string ipList = util::cfgpath(util::Data) + "/ip_list.txt";
  QFile file(ipList.c_str()); // List of IP's now defined in a file to avoid recompiles !
  QString line;
  if (file.open(QIODevice::ReadOnly) ) {       
    QTextStream t( &file );        // use a text stream
    while ( !t.atEnd() ) {           
      line = t.readLine();         // line of text excluding '\n'
      //locationBox->addItem("10.0.0.21");
      locationBox->addItem(line);

    }
    // Close the file
    file.close();
  }

  connect (updateTime, SIGNAL(toggled(bool)), (UTMainWnd*)parent, SLOT(saveConfig(bool)));
  connect (locationBox, SIGNAL(currentIndexChanged(int)), (UTMainWnd*)parent, SLOT(saveConfig(int)));
  
  connect (initialButton, SIGNAL(clicked()), SLOT(setInitial()));
  connect (readyButton, SIGNAL(clicked()), SLOT(setReady()));
  connect (setButton, SIGNAL(clicked()), SLOT(setSet()));
  connect (playingButton, SIGNAL(clicked()), SLOT(setPlaying()));
  connect (testingButton, SIGNAL(clicked()), SLOT(setTesting()));
  connect (penalisedButton, SIGNAL(clicked()), SLOT(setPenalised()));
  connect (finishedButton, SIGNAL(clicked()), SLOT(setFinished()));
  //connect (topCamButton, SIGNAL(clicked()), SLOT(setTopCameraBehavior()));
  //connect (botCamButton, SIGNAL(clicked()), SLOT(setBottomCameraBehavior()));
  connect (testOdometryButton, SIGNAL(clicked()), SLOT(setTestOdometry()));
  
  connect (restartPythonButton, SIGNAL(clicked()), parent, SLOT(remoteRestartInterpreter()));
  connect (uploadButton, SIGNAL(clicked()), this, SLOT(sendPython()));
  connect (upRobotCfgButton, SIGNAL(clicked()), this, SLOT(sendRobotConfig()));
  //connect (verifyConfigButton, SIGNAL(clicked()), this, SLOT(verifyRobotConfig()));

  connect (downLogsButton, SIGNAL(clicked()), this, SLOT(getLogs()));
  connect (removeLogsButton, SIGNAL(clicked()), this, SLOT(removeLogs()));

  connect (upBinaryButton, SIGNAL(clicked()), this, SLOT(sendBinary()));
  connect (upAllButton, SIGNAL(clicked()), this, SLOT(sendAll()));

  connect (upColorButton, SIGNAL(clicked()), this, SLOT(sendColorTable()));
  connect (upVisionButton, SIGNAL(clicked()), this, SLOT(sendVision()));
  connect (upMotionButton, SIGNAL(clicked()), this, SLOT(sendMotion()));
  connect (upNaoButton, SIGNAL(clicked()), this, SLOT(sendInterface()));
  connect (upCfgButton, SIGNAL(clicked()), this, SLOT(sendConfigFiles()));

  connect (verifyEveryButton, SIGNAL(clicked()), this, SLOT(verifyAll()));

  connect (restartNaoQiButton, SIGNAL(clicked()), this, SLOT(restartNaoQi()));
  //connect (mp3Button, SIGNAL(clicked()), this, SLOT(sendMp3Files()));
  connect (locationBox, SIGNAL(currentIndexChanged(int)), this, SLOT(locationChanged(int)));
  connect (resetTopButton, SIGNAL(clicked()), this, SLOT(resetTopCamera()));
  connect (resetBottomButton, SIGNAL(clicked()), this, SLOT(resetBottomCamera()));

  connect (stopNaoqiButton, SIGNAL(clicked()), this, SLOT(stopNaoqi()));
  connect (startNaoqiButton, SIGNAL(clicked()), this, SLOT(startNaoqi()));
  connect (this, SIGNAL(robotStatusUpdated(bool)), this, SLOT(updateRobotStatus(bool)));

  connect (this, SIGNAL(statusUpdated(QString)), filesStatus, SLOT(showMessage(QString)));

  connect (behaviorButton, SIGNAL(clicked()), this, SLOT(runBehavior()));

  locationBox->setCurrentIndex(0);
  loadBehaviors();
  behaviorBox->setCurrentIndex(behaviorBox->findText("sample"));
}

ProcessExecutor::Callback FilesWindow::getStatusCallback(QString message) {
  return [=](bool) { statusUpdated(message); };
}

void FilesWindow::loadConfig(const ToolConfig& config) {
  if(locationBox->count() > config.filesLocationIndex)
    locationBox->setCurrentIndex(config.filesLocationIndex);
  updateTime->setChecked(config.filesUpdateTimeChecked);
}

void FilesWindow::saveConfig(ToolConfig& config) {
  config.filesLocationIndex = locationBox->currentIndex();
  config.filesUpdateTimeChecked = updateTime->isChecked();
}

void FilesWindow::setInitial() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateInitial);
}

void FilesWindow::setReady() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateReady);
}

void FilesWindow::setSet() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateSet);
}

void FilesWindow::setPlaying() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StatePlaying);
}

void FilesWindow::setPenalised() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StatePenalized);
}

void FilesWindow::setFinished() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateFinished);
}

void FilesWindow::setTesting() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateTesting);
}

void FilesWindow::setBottomCameraBehavior() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateCameraBottom);
}

void FilesWindow::setTopCameraBehavior() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::StateCameraTop);
}

void FilesWindow::loadBehaviors() {
  QString path = QString(getenv("NAO_HOME")) + "/core/python/behaviors";
  QStringList filters;
  filters << "*.py";
  QDir dir(path);
  QStringList files = dir.entryList(filters);
  QStringList behaviors;
  for(auto file : files) {
    file = file.left(file.lastIndexOf("."));
    if(file == "__init__") continue;
    behaviors << file;
  }
  behaviorBox->clear();
  behaviorBox->addItems(behaviors);
}

void FilesWindow::runBehavior() {
  std::string behavior = behaviorBox->currentText().toStdString();
  ToolPacket tp(ToolPacket::RunBehavior, behavior);
  UTMainWnd::inst()->sendUDPCommand(locationBox->currentText(), tp);
}

void FilesWindow::setTestOdometry() {
  ToolPacket tp(ToolPacket::StateTestOdometry);
  tp.odom_command.x = odom_fwd->value();
  tp.odom_command.y = odom_side->value();
  tp.odom_command.theta = odom_turn->value();
  tp.odom_command.time = odom_time->value();
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), tp);
}

void FilesWindow::resetTopCamera() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::ResetCameraParameters);
}

void FilesWindow::resetBottomCamera() {
  ((UTMainWnd*)parent)->sendUDPCommand(locationBox->currentText(), ToolPacket::ResetCameraParameters);
}

void FilesWindow::stopNaoqi() {
  naoqiCommand("stop");
}

void FilesWindow::startNaoqi() {
  naoqiCommand("start");
}

void FilesWindow::sendCopyRobotCommand(QString command, bool verbose) {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  auto func = [=] (bool returnState) {
    auto message = command;
    message += (returnState ? " succeeded " : " failed ");
    message += "for " + address;
    statusUpdated(message);
  };
  executor_.sendCopyRobotCommand(address, command, verbose, func);
}

void FilesWindow::sendPython(bool verbose) {
  std::cout << "-- Sending python --" << std::endl;
  sendCopyRobotCommand("python",verbose);
}

void FilesWindow::verifyPython(bool verbose) {
  std::cout << "-- Verifying python --" << std::endl;
  sendCopyRobotCommand("python --verify",verbose);
}

void FilesWindow::sendBinary(bool verbose) {
  std::cout << "-- Sending binaries --" << std::endl;
  sendCopyRobotCommand("nao motion vision",verbose);
}

void FilesWindow::verifyBinary(bool verbose) {
  std::cout << "-- Verifying binaries --" << std::endl;
  sendCopyRobotCommand("nao motion vision --verify",verbose);
}

void FilesWindow::sendAll(bool verbose) {
  std::cout << "-- Sending all --" << std::endl;
  sendCopyRobotCommand("all",verbose);
}

void FilesWindow::verifyAll(bool verbose) {
  std::cout << "-- Verify all --" << std::endl;
  sendCopyRobotCommand("all --verify",verbose);
}

void FilesWindow::sendVision(bool verbose) {
  std::cout << "-- Sending vision --" << std::endl;
  sendCopyRobotCommand("vision",verbose);
}

void FilesWindow::verifyVision(bool verbose) {
  std::cout << "-- Verifying vision --" << std::endl;
  sendCopyRobotCommand("vision --verify",verbose);
}

void FilesWindow::sendMotion(bool verbose) {
  std::cout << "-- Sending motion --" << std::endl;
  sendCopyRobotCommand("motion",verbose);
}

void FilesWindow::verifyMotion(bool verbose) {
  std::cout << "-- Verifying motion --" << std::endl;
  sendCopyRobotCommand("motion --verify",verbose);
}

void FilesWindow::sendInterface(bool verbose) {
  std::cout << "-- Sending nao interface --" << std::endl;
  sendCopyRobotCommand("nao",verbose);
}

void FilesWindow::verifyInterface(bool verbose) {
  std::cout << "-- Verifying nao interface --" << std::endl;
  sendCopyRobotCommand("nao --verify",verbose);
}

void FilesWindow::sendColorTable(bool verbose) {
  std::cout << "-- Sending color tables --" << std::endl;
  sendCopyRobotCommand("color_table",verbose);
}

void FilesWindow::verifyColorTable(bool verbose) {
  std::cout << "-- Verifying color tables --" << std::endl;
  sendCopyRobotCommand("color_table --verify",verbose);
}

void FilesWindow::sendConfigFiles(bool verbose) {
  std::cout << "-- Sending config files --" << std::endl;
  sendCopyRobotCommand("configs",verbose);
  sendCopyRobotCommand("scripts",verbose);
}

void FilesWindow::verifyConfigFiles(bool verbose) {
  std::cout << "-- Verifying config files --" << std::endl;
  sendCopyRobotCommand("configs --verify",verbose);
  sendCopyRobotCommand("scripts --verify",verbose);
}

void FilesWindow::sendRobotConfig(bool verbose) {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  RobotConfig config;
  config.team = teamNumBox->value();
  config.self = roleBox->value();
  config.robot_id = address.section('.', 3).toInt();
  executor_.sendRobotConfig(address, config, verbose);
}

void FilesWindow::verifyRobotConfig(bool verbose) {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  RobotConfig config;
  config.team = teamNumBox->value();
  config.self = roleBox->value();
  config.robot_id = address.section('.', 3).toInt();
  executor_.verifyRobotConfig(address, config, verbose);
}

void FilesWindow::sendFile(QString from, QString to, QString name){
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  // make sure it is a robot
  if (address == "localhost") {
    cout << "Error, attempt to copy to localhost" << endl;
    filesStatus->showMessage("Error, attempt to copy to localhost");
    return;
  }

  cout << "Copying " << name.toStdString() << " to " << address.toStdString() << " ... " << flush;
  filesStatus->showMessage("Copying "+name+ " to " + address);
  
  QProcess *scp;
  scp = new QProcess( this ); // memory allocation from heap, created with parent
  
  QString out = "nao@";
  out.append(address + ":" + to);
  QStringList cmd;
  cmd.push_back(QString("-avz"));
  cmd.push_back(from);
  cmd.push_back(out);
  scp->start("rsync", cmd);
  if (!scp->waitForStarted()) {
    cout << "sendLib Error 1" << endl << flush;
    return;
  }
  std::cout << cmd.join(" ").toStdString() << std::endl;

  scp->closeWriteChannel();

  scp->waitForFinished();

  QByteArray result = scp->readAll();
  cout << "Done!\n" << endl;
  filesStatus->showMessage(name +" copied to " + address);
}  

void FilesWindow::sendMp3Files() {
  
  sendFile(dataPath + "fight.mp3","~/data","fight.mp3");
  sendFile(dataPath + "eyes.mp3","~/data","eyes.mp3");

}

void FilesWindow::naoqiCommand(QString c) {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  // make sure it is a robot
  if (address == "localhost") {
    cout << ("Error, attempt to " + c + " localhost").toStdString() << endl;
    filesStatus->showMessage("Error, attempt to " + c + " localhost");
    return;
  }
  executor_.sendNaoqiCommand(address, c);
}

void FilesWindow::restartNaoQi() {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  executor_.restartNaoqi(address);
}

void FilesWindow::setRobotTime() {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  executor_.setRobotTime(address);
}

void FilesWindow::getLogs() {

  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  // make sure it is a robot
  if (address == "localhost") {
    cout << "Error, attempt to copy to localhost" << endl;
    filesStatus->showMessage("Error, attempt to copy to localhost");
    return;
  }
  executor_.getLogs(address, getStatusCallback("Done getting logs!"));
}

void FilesWindow::removeLogs(){

  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  // make sure it is a robot
  if (address == "localhost") {
    cout << "Error, attempt to copy to localhost" << endl;
    filesStatus->showMessage("Error, attempt to copy to localhost");
    return;
  }
  executor_.removeLogs(address, getStatusCallback("Done removing logs!"));
}

void FilesWindow::updateRobotStatus(bool alive) {
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  whichRobot->setText(address);
  robotStatus->setAutoFillBackground(true);
  QPalette rspalette;
  if (alive) {
    rspalette.setColor(QPalette::WindowText, Qt::darkGreen);
    robotStatus->setText("Alive");
    if(!isRobotTimeSet && updateTime->isChecked()) {
      setRobotTime();
      isRobotTimeSet = true;
    }
  }
  else {
    isRobotTimeSet = false;
    rspalette.setColor(QPalette::WindowText, Qt::darkRed);
    robotStatus->setText("Dead");
  }
  robotStatus->setPalette(rspalette);
}

void FilesWindow::checkStatus(bool repeat){
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  
  if (!address.contains("core")) {
    auto callback = [=] (bool status) {
      robotStatusUpdated(status);
    };
    executor_.checkRobotStatus(address, callback);
  }
  // check every 10 seconds
  if(repeat)
    statusTimer->singleShot(10 * 1000,this,SLOT(checkStatus()));
}

// enable or disable the buttons
void FilesWindow::enableButtons(bool b){

  restartPythonButton->setEnabled(b);
  uploadButton->setEnabled(b);
  upRobotCfgButton->setEnabled(b);

  downLogsButton->setEnabled(b);
  removeLogsButton->setEnabled(b);

  upBinaryButton->setEnabled(b);
  upAllButton->setEnabled(b);
  upColorButton->setEnabled(b);

  restartNaoQiButton->setEnabled(b);
  stopNaoqiButton->setEnabled(b);
  startNaoqiButton->setEnabled(b);

  upVisionButton->setEnabled(b);
  upMotionButton->setEnabled(b);
  upNaoButton->setEnabled(b);
  upCfgButton->setEnabled(b);

  verifyEveryButton->setEnabled(b);
  //verifyConfigButton->setEnabled(b);

  /*
    mp3Button->setEnabled(b);
  */
}



// called from control window
void FilesWindow::changeLocationIndex(int index) {
  // change location index
  locationBox->setCurrentIndex(index);
}

void FilesWindow::locationChanged(int index){
  // update address/status
  QString address = ((UTMainWnd*)parent)->getCurrentAddress();
  whichRobot->setText(address);
  QPalette rspalette;
  robotStatus->setText("Unknown");
  rspalette.setColor(QPalette::WindowText, Qt::darkYellow);
  robotStatus->setPalette(rspalette);


  // update log select window
  UTMainWnd::inst()->logSelectWnd_->updateSelectedIP(address);
  UTMainWnd::inst()->addressChanged();
  checkStatus(false);
}

void FilesWindow::setCurrentLocation(std::string ip) {
  setCurrentLocation(QString::fromStdString(ip));
}

void FilesWindow::setCurrentLocation(QString ip) {
  int index = locationBox->findText(ip);
  if (index == -1){
    locationBox->insertItem(0, ip);
    index = 0;
  }
  locationBox->setCurrentIndex(index);
}
