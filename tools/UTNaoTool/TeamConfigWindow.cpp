// This now works by setting the appropriate IP in the control window and then calling the Files window methods
// for scp'ing files


#include <QtGui>
#include "TeamConfigWindow.h"
#include "UTMainWnd.h"

#include "LogSelectWindow.h"
#include "FilesWindow.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <common/WorldObject.h>
#include <common/Util.h>
#include <memory/LogWriter.h>

using namespace std;

TeamConfigWindow::TeamConfigWindow(QMainWindow* p) {
  parent = p;

  setupUi(this);
  setWindowTitle(tr("Team Config"));
  connect (reloadConfigButton, SIGNAL(clicked()), this, SLOT(reloadLocalConfig()));
  connect (saveConfigButton, SIGNAL(clicked()), this, SLOT(saveLocalConfig()));

  connect (stopNaoQiButton, SIGNAL(clicked()), this, SLOT(stopNaoQi()));
  connect (startNaoQiButton, SIGNAL(clicked()), this, SLOT(startNaoQi()));
  connect (restartInterpreterButton, SIGNAL(clicked()), this, SLOT(restartInterpreter()));
  connect (uploadEverythingButton, SIGNAL(clicked()), this, SLOT(uploadEverything()));
  connect (uploadBinaryButton, SIGNAL(clicked()), this, SLOT(uploadBinary()));
  connect (uploadMotionVisionConfigButton, SIGNAL(clicked()), this, SLOT(uploadMotionVisionConfig()));
  connect (uploadRobotConfigButton, SIGNAL(clicked()), this, SLOT(uploadRobotConfig()));
  connect (uploadColorButton, SIGNAL(clicked()), this, SLOT(uploadColor()));
  connect (uploadInterpreterButton, SIGNAL(clicked()), this, SLOT(uploadInterpreter()));
  connect (uploadTimeButton, SIGNAL(clicked()), this, SLOT(uploadTime()));
  connect (this, SIGNAL(robotStatusUpdated(int,ProcessExecutor::RobotStatus)), this, SLOT(updateRobotStatus(int, ProcessExecutor::RobotStatus)));
  connect (openSSH, SIGNAL(clicked()), this, SLOT(updateSSH()));
  
  connect (this, SIGNAL(logStatusUpdated(int,QString)), this, SLOT(updateLogStatus(int,QString)));
  connect (logStartButton, SIGNAL(clicked()), this, SLOT(startLogging()));
  connect (logStopButton, SIGNAL(clicked()), this, SLOT(stopLogging()));
  connect (logCopyButton, SIGNAL(clicked()), this, SLOT(copyLogs()));
  connect (logDeleteButton, SIGNAL(clicked()), this, SLOT(deleteLogs()));

  // set to our rc 2013 team # (1)
  GCNum->setValue(1);

  // check on robot's status
  // start timer for status updates
  QTimer *timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(checkStatus()));
  // check on robots using fping every 15 seconds
  timer->start(15000);
  posX_ = {
    { WO_TEAM1, x1 },
    { WO_TEAM2, x2 },
    { WO_TEAM3, x3 },
    { WO_TEAM4, x4 },
    { WO_TEAM5, x5 },
    { WO_TEAM_COACH, coachX }
  };
  posY_ = {
    { WO_TEAM1, y1 },
    { WO_TEAM2, y2 },
    { WO_TEAM3, y3 },
    { WO_TEAM4, y4 },
    { WO_TEAM5, y5 },
    { WO_TEAM_COACH, coachY }
  };
  posT_ = {
    { WO_TEAM1, t1 },
    { WO_TEAM2, t2 },
    { WO_TEAM3, t3 },
    { WO_TEAM4, t4 },
    { WO_TEAM5, t5 },
    { WO_TEAM_COACH, coachT }
  };
  logStatuses_ = {
    { WO_TEAM1, lstat1 },
    { WO_TEAM2, lstat2 },
    { WO_TEAM3, lstat3 },
    { WO_TEAM4, lstat4 },
    { WO_TEAM5, lstat5 },
    { WO_TEAM_COACH, lstatCoach }
  };
  ipboxes_ = {
    std::make_pair(WO_TEAM1, IP1),
    std::make_pair(WO_TEAM2, IP2),
    std::make_pair(WO_TEAM3, IP3),
    std::make_pair(WO_TEAM4, IP4),
    std::make_pair(WO_TEAM5, IP5),
    std::make_pair(WO_TEAM_COACH, IPCoach)
  };
  checks_ = {
    std::make_pair(WO_TEAM1, IPBox1),
    std::make_pair(WO_TEAM2, IPBox2),
    std::make_pair(WO_TEAM3, IPBox3),
    std::make_pair(WO_TEAM4, IPBox4),
    std::make_pair(WO_TEAM5, IPBox5),
    std::make_pair(WO_TEAM_COACH, IPBoxCoach)
  };
  statuses_ = {
    std::make_pair(WO_TEAM1, IP1_status),
    std::make_pair(WO_TEAM2, IP2_status),
    std::make_pair(WO_TEAM3, IP3_status),
    std::make_pair(WO_TEAM4, IP4_status),
    std::make_pair(WO_TEAM5, IP5_status),
    std::make_pair(WO_TEAM_COACH, IPCoach_status)
  };
  robots_ = { WO_TEAM1, WO_TEAM2, WO_TEAM3, WO_TEAM4, WO_TEAM5, WO_TEAM_COACH };
}

void TeamConfigWindow::reloadLocalConfig() {
  std::string color = redButton->isChecked() ? "red" : "blue";
  std::string path = std::string(getenv("NAO_HOME")) + "/data/team_config_" + color + ".yaml";
  TeamConfig tconfig;
  if(tconfig.loadFromFile(path)) {
    for(int i : robots_) {
      RobotConfig rconfig = tconfig.robot_configs[i];
      tconfig.robot_configs[i] = getRobotConfig(i);
      auto box = ipboxes_[i];
      if(!rconfig.robot_id) {
        box->setText("");
        continue;
      }
      box->setText(QString::number(rconfig.robot_id));
      GCNum->setValue(rconfig.team);
      //gcIP->setText(QString::fromStdString(rconfig.game_controller_ip));
      auto px = posX_[i], py = posY_[i], pt = posT_[i];
      px->setValue(rconfig.posX);
      py->setValue(rconfig.posY);
      pt->setValue(rconfig.orientation);
      if(rconfig.self == WO_TEAM_COACH) {
        coachZ->setValue(rconfig.posZ);
      }
      tbIP->setText(QString::fromStdString(rconfig.team_broadcast_ip));
      tUDP->setValue(rconfig.team_udp);
    }
    commonIP->setText(QString::fromStdString(tconfig.common_ip));
    audioEnabled->setChecked(tconfig.audio_enabled);
  } else {
    cout << "Team Manager Window: No config to load" << std::endl;
  }
}


void TeamConfigWindow::saveLocalConfig() {
  std::string color = redButton->isChecked() ? "red" : "blue";
  std::string path = std::string(getenv("NAO_HOME")) + "/data/team_config_" + color + ".yaml";
  TeamConfig tconfig;
  for(auto i : robots_) {
    tconfig.robot_configs[i] = getRobotConfig(i);
  }
  tconfig.common_ip =commonIP->text().toStdString();
  tconfig.audio_enabled = audioEnabled->isChecked();
  tconfig.saveToFile(path);
  cout << "Team Manager Window: Config Written to " << path << "\n";
}

RobotConfig TeamConfigWindow::getRobotConfig(int self) {
  RobotConfig config;
  QLineEdit* ipbox = ipboxes_[self];
  if(ipbox->text() == "") return config;
  config.robot_id = ipbox->text().toInt();
  //config.game_controller_ip = gcIP->text().toStdString();
  auto px = posX_[self], py = posY_[self], pt = posT_[self];
  config.posX = px->value();
  config.posY = py->value();
  config.orientation = pt->value();
  if(self == WO_TEAM_COACH) {
    config.posZ = coachZ->value();
  }
  config.team = GCNum->value();
  config.self = config.role = self;
  config.team_broadcast_ip = tbIP->text().toStdString();
  config.team_udp = tUDP->value();
  config.audio_enabled = audioEnabled->isChecked();
  return config;
}


QStringList TeamConfigWindow::getUploadList() {
  QStringList list;
  if (IP1->text()!="" && IPBox1->isChecked()) list.append(IP1->text());
  if (IP2->text()!="" && IPBox2->isChecked()) list.append(IP2->text());
  if (IP3->text()!="" && IPBox3->isChecked()) list.append(IP3->text());
  if (IP4->text()!="" && IPBox4->isChecked()) list.append(IP4->text());
  if (IP5->text()!="" && IPBox5->isChecked()) list.append(IP5->text());
  if (IP6->text()!="" && IPBox6->isChecked()) list.append(IP6->text());
  if (IPCoach->text()!="" && IPBoxCoach->isChecked()) list.append(IPCoach->text());
  return list;
}

void TeamConfigWindow::startNaoQi() {
  cout << "Team Manager Window: Starting NaoQi's\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]);
    executor_.startNaoqi(ip);
  }
}

void TeamConfigWindow::stopNaoQi() {
  cout << "Team Manager Window: Stopping NaoQi's\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]);
    executor_.stopNaoqi(ip);
  }
}

void TeamConfigWindow::restartInterpreter() {
  cout << "Team Manager Window: Restarting Interpreter\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();

    // set it in control window and call files restartInterpreter on it
    ((UTMainWnd*)parent)->filesWnd_->setCurrentLocation(ip);
    ((UTMainWnd*)parent)->remoteRestartInterpreter();

  }
}



void TeamConfigWindow::uploadEverything() {
  uploadBinary();
  uploadInterpreter();
  uploadMotionVisionConfig();
  uploadColor();
  uploadRobotConfig();
  //uploadWireless();
  //TODO: callback on complete
}

void TeamConfigWindow::uploadBinary() {

  //cout << "Team Manager Window: Upload Binary\n";
  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();
    executor_.sendBinary(ip, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked())
        executor_.verifyBinaryOld(ip, false);
      else
        executor_.verifyBinary(ip, false);
    }
  }
}

void TeamConfigWindow::uploadMotionVisionConfig() {
  //cout << "Team Manager Window: Upload Config\n";
  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = list[i]; //toLatin1().data();
    // make config file

    ip = getFullIP(ip);
    executor_.sendConfigFiles(ip, false);
    executor_.sendMotionFiles(ip, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked()) {
        executor_.verifyConfigFilesOld(ip, false);
        executor_.verifyMotionFilesOld(ip, false);
      }
      else {
        executor_.verifyConfigFiles(ip, false);
        executor_.verifyMotionFiles(ip, false);
      }
    }
  }
}

void TeamConfigWindow::uploadRobotConfig() {
  for(auto robot : robots_) {
    auto check = checks_[robot];
    auto box = ipboxes_[robot];
    if(box->text() == "") continue;
    if(!check->isChecked()) continue;
    QString ip = getFullIP(box->text());
    RobotConfig config = getRobotConfig(robot);
    executor_.sendRobotConfig(ip, config, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked())
        executor_.verifyRobotConfigOld(ip, config, false);
      else
        executor_.verifyRobotConfig(ip, config, false);
    }
  }
}

void TeamConfigWindow::uploadColor() {
  //cout << "Team Manager Window: Upload Color Table\n";
  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();

    executor_.sendColorTable(ip, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked())
        executor_.verifyColorTableOld(ip, false);
      else
        executor_.verifyColorTable(ip, false);
    }
  }
}

void TeamConfigWindow::uploadWireless() {
  cout << "Team Manager Window: Upload Wireless\n";
  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();
    //executor_.sendWireless(ip, false);
  }
}

void TeamConfigWindow::uploadInterpreter() {
  //cout << "Team Manager Window: Upload Py/Lua\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();

    executor_.sendLua(ip, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked())
        executor_.verifyLuaOld(ip, false);
      else
        executor_.verifyLua(ip, false);
    }
    executor_.sendPython(ip, false);
    if(verifyEnabled->isChecked()) {
      if(!verifyNew->isChecked())
        executor_.verifyPythonOld(ip, false);
      else
        executor_.verifyPython(ip, false);
    }
  }
}

void TeamConfigWindow::checkStatus(){

  // only if this window is active
  if (!this->isVisible())
    return;
  updateSSH();

  //for(auto robot : robots_) {
    //auto box = ipboxes_[robot];
    //if(box->text() == "") continue;
    //QString ip = getFullIP(box->text());
    //clearRobotStatus(robot);
    //auto callback = [=] (ProcessExecutor::RobotStatus status) {
      //robotStatusUpdated(robot, status);
    //};
    //executor_.checkRobotStatus(ip, callback);
  //}
}

void TeamConfigWindow::clearRobotStatus(int robot) {
  auto box = ipboxes_[robot];
  QString ip = getFullIP(box->text());
  auto label = statuses_[robot];
  label->setText("-");
}

void TeamConfigWindow::updateRobotStatus(int robot, ProcessExecutor::RobotStatus status) {
  auto box = ipboxes_[robot];
  QString ip = getFullIP(box->text());
  auto label = statuses_[robot];
  switch(status) {
    case ProcessExecutor::Connecting: label->setText("C..."); break;
    case ProcessExecutor::Connected: label->setText("CE"); break;
    case ProcessExecutor::Alive: label->setText("A"); break;
    default: label->setText("xxx"); break;
  }
}

void TeamConfigWindow::uploadTime() {
  auto list = getUploadList();
  int size=list.size();
  std::cout << "TIME" << std::endl;
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();
    executor_.setRobotTime(ip);
  }
}

QString TeamConfigWindow::getFullIP(const QString &suffix) {
  QString common = commonIP->text();
  if (!common.endsWith("."))
    common += ".";
  return common + suffix;
}

void TeamConfigWindow::updateSSH() {
  for(auto robot : robots_) {
    auto box = ipboxes_[robot];
    if(box->text() == "") continue;
    QString ip = getFullIP(box->text());
    auto callback = [=] (ProcessExecutor::RobotStatus status) {
      robotStatusUpdated(robot, status);
    };
    if(openSSH->isChecked())
      executor_.openSSH(ip, callback);
    else
      executor_.closeSSH(ip, callback);
  }
}

void TeamConfigWindow::updateLogStatus(int robot, QString status) {
  auto slabel = logStatuses_[robot];
  slabel->setText(status);
}

void TeamConfigWindow::startLogging() {
  auto list = getUploadList();
  vector<QString> ips;
  for(int i = 0; i < list.size(); i++) {
    auto robot = robots_[i];
    ips.push_back(getFullIP(list[i]));
    logStatusUpdated(robot, "On");
  }
  LogSelectWindow::inst()->startMultiLogging(ips);
}

void TeamConfigWindow::stopLogging() {
  auto list = getUploadList();
  vector<QString> ips;
  map<QString,int> lookup;
  for(int i = 0; i < list.size(); i++) {
    auto robot = robots_[i];
    auto ip = getFullIP(list[i]);
    lookup[ip] = robot;
    ips.push_back(ip);
    logStatusUpdated(robot, "?");
  }
  auto callback = [=] (QString ip) mutable {
    int robot = lookup[ip];
    logStatusUpdated(robot, "Off");
  };
  LogSelectWindow::inst()->stopMultiLogging(ips, callback);
}

void TeamConfigWindow::copyLogs() {
  auto list = getUploadList();
  for(int i = 0; i < list.size(); i++) {
    QString destination = getenv("NAO_HOME");
    destination += "/logs/team/" + QString::number(i);
    util::mkdir_recursive(destination.toStdString().c_str());
    auto robot = robots_[i];
    auto ip = getFullIP(list[i]);
    auto callback = [=](bool result) {
      printf("Logs deleted from %s\n", ip.toStdString().c_str());
    };
    executor_.getLogs(ip, destination, callback);
  }
}

void TeamConfigWindow::deleteLogs() {
  auto list = getUploadList();
  for(int i = 0; i < list.size(); i++) {
    auto robot = robots_[i];
    auto ip = getFullIP(list[i]);
    auto callback = [=](bool result) {
      printf("Logs deleted from %s\n", ip.toStdString().c_str());
    };
    executor_.removeLogs(ip, callback);
  }
}
