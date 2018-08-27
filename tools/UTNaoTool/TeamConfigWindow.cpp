// This now works by setting the appropriate IP in the control window and then calling the Files window methods
// for scp'ing files


#include <QtGui>
#include <tool/TeamConfigWindow.h>
#include <tool/UTMainWnd.h>

#include <tool/LogSelectWindow.h>
#include <tool/FilesWindow.h>

#include <iostream>
#include <common/WorldObject.h>
#include <memory/LogWriter.h>

#define WO_ALT1 (WO_TEAM_LAST + 1)
#define WO_ALT2 (WO_ALT1 + 1)

#define status_callback std::bind(&TeamConfigWindow::statusCallback, this, std::placeholders::_1)
#define MAP(self) (self > WO_TEAM_LAST && self < WO_TEAM_COACH ? WO_TEAM_LAST : self)

using namespace std;

TeamConfigWindow::TeamConfigWindow(QMainWindow* p) {
  parent = p;
  qRegisterMetaType<TeamConfigWindow::UploadStatus>("TeamConfigWindow::UploadStatus");

  setupUi(this);
  setWindowTitle(tr("Team Config"));
  connect (reloadConfigButton, SIGNAL(clicked()), this, SLOT(reloadLocalConfig()));
  connect (saveConfigButton, SIGNAL(clicked()), this, SLOT(saveLocalConfig()));

  connect (stopNaoQiButton, SIGNAL(clicked()), this, SLOT(stopNaoQi()));
  connect (startNaoQiButton, SIGNAL(clicked()), this, SLOT(startNaoQi()));
  connect (restartInterpreterButton, SIGNAL(clicked()), this, SLOT(restartInterpreter()));
  connect (compileEverythingButton, SIGNAL(clicked()), this, SLOT(compileEverything()));
  connect (uploadEverythingButton, SIGNAL(clicked()), this, SLOT(uploadEverything()));
  connect (uploadBinaryButton, SIGNAL(clicked()), this, SLOT(uploadBinary()));
  connect (uploadMotionVisionConfigButton, SIGNAL(clicked()), this, SLOT(uploadMotionVisionConfig()));
  connect (uploadRobotConfigButton, SIGNAL(clicked()), this, SLOT(uploadRobotConfig()));
  connect (uploadColorButton, SIGNAL(clicked()), this, SLOT(uploadColor()));
  connect (uploadInterpreterButton, SIGNAL(clicked()), this, SLOT(uploadInterpreter()));
  connect (this, SIGNAL(robotStatusUpdated(int,ProcessExecutor::RobotStatus)), this, SLOT(updateRobotStatus(int, ProcessExecutor::RobotStatus)));
  connect (this, SIGNAL(uploadStatusUpdated(TeamConfigWindow::UploadStatus)), this, SLOT(updateUploadStatus(TeamConfigWindow::UploadStatus)));
  connect (openSSH, SIGNAL(clicked()), this, SLOT(updateSSH()));
  
  connect (this, SIGNAL(logStatusUpdated(int,QString)), this, SLOT(updateLogStatus(int,QString)));
  connect (logStartButton, SIGNAL(clicked()), this, SLOT(startLogging()));
  connect (logStopButton, SIGNAL(clicked()), this, SLOT(stopLogging()));
  connect (logCopyButton, SIGNAL(clicked()), this, SLOT(copyLogs()));
  connect (logDeleteButton, SIGNAL(clicked()), this, SLOT(deleteLogs()));

  // set to our rc 2013 team # (1)
  team_number->setValue(1);

  // check on robot's status
  // start timer for status updates
  QTimer *timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(checkStatus()));
  // check on robots using fping every 5 seconds
  timer->start(5000);
  posX_ = {
    { WO_TEAM1, x_1 },
    { WO_TEAM2, x_2 },
    { WO_TEAM3, x_3 },
    { WO_TEAM4, x_4 },
    { WO_TEAM5, x_5 },
    { WO_TEAM_COACH, x_c },
    { WO_ALT1, x_a1 },
    { WO_ALT2, x_a2 }
  };
  posY_ = {
    { WO_TEAM1, y_1 },
    { WO_TEAM2, y_2 },
    { WO_TEAM3, y_3 },
    { WO_TEAM4, y_4 },
    { WO_TEAM5, y_5 },
    { WO_TEAM_COACH, y_c },
    { WO_ALT1, y_a1 },
    { WO_ALT2, y_a2 }
  };
  posT_ = {
    { WO_TEAM1, t_1 },
    { WO_TEAM2, t_2 },
    { WO_TEAM3, t_3 },
    { WO_TEAM4, t_4 },
    { WO_TEAM5, t_5 },
    { WO_TEAM_COACH, t_c },
    { WO_ALT1, t_a1 },
    { WO_ALT2, t_a2 }
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
    { WO_TEAM1, ip_1}, 
    { WO_TEAM2, ip_2}, 
    { WO_TEAM3, ip_3}, 
    { WO_TEAM4, ip_4}, 
    { WO_TEAM5, ip_5}, 
    { WO_TEAM_COACH, ip_c}, 
    { WO_ALT1, ip_a1 },
    { WO_ALT2, ip_a2 }
  };
  sonars_ = {
    { WO_TEAM1, sonar_1}, 
    { WO_TEAM2, sonar_2}, 
    { WO_TEAM3, sonar_3}, 
    { WO_TEAM4, sonar_4}, 
    { WO_TEAM5, sonar_5}, 
    { WO_TEAM_COACH, sonar_c}, 
    { WO_ALT1, sonar_a1 },
    { WO_ALT2, sonar_a2 }
  };
  for(auto& kvp : ipboxes_)
    connect(kvp.second, SIGNAL(editingFinished()), this, SLOT(updateSSH()));
  
  checks_ = {
    { WO_TEAM1, select_1}, 
    { WO_TEAM2, select_2}, 
    { WO_TEAM3, select_3}, 
    { WO_TEAM4, select_4}, 
    { WO_TEAM5, select_5}, 
    { WO_TEAM_COACH, select_c}, 
    { WO_ALT1, select_a1 },
    { WO_ALT2, select_a2 }
  };
  for(auto& kvp : checks_)
    connect(kvp.second, SIGNAL(toggled(bool)), this, SLOT(updateSSH(bool)));

  statuses_ = {
    { WO_TEAM1, status_1}, 
    { WO_TEAM2, status_2}, 
    { WO_TEAM3, status_3}, 
    { WO_TEAM4, status_4}, 
    { WO_TEAM5, status_5}, 
    { WO_TEAM_COACH, status_c}, 
    { WO_ALT1, status_a1 },
    { WO_ALT2, status_a2 }
  };
  robots_ = { WO_TEAM1, WO_TEAM2, WO_TEAM3, WO_TEAM4, WO_TEAM5, WO_TEAM_COACH, WO_ALT1, WO_ALT2 };
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
      auto px = posX_[i], py = posY_[i], pt = posT_[i];
      auto sonar = sonars_[i];
      px->setValue(rconfig.posX);
      py->setValue(rconfig.posY);
      pt->setValue(rconfig.orientation);
      sonar->setChecked(rconfig.sonar_enabled);
      if(rconfig.self == WO_TEAM_COACH) {
        z_coach->setValue(rconfig.posZ);
      }
    }
    ip_common->setText(QString::fromStdString(tconfig.common_ip));
    audioEnabled->setChecked(tconfig.audio_enabled);
    svmEnabled->setChecked(tconfig.svm_enabled);
    optimizeEnabled->setChecked(tconfig.optimize_enabled);
    ip_broadcast->setText(QString::fromStdString(tconfig.team_broadcast_ip));
    team_udp->setValue(tconfig.team_udp);
    team_number->setValue(tconfig.team);
    ip_game_controller->setText(QString::fromStdString(tconfig.game_controller_ip));
    updateSSH();
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
  tconfig.common_ip = ip_common->text().toStdString();
  tconfig.audio_enabled = audioEnabled->isChecked();
  tconfig.svm_enabled = svmEnabled->isChecked();
  tconfig.optimize_enabled = optimizeEnabled->isChecked();
  tconfig.team_broadcast_ip = ip_broadcast->text().toStdString();
  tconfig.game_controller_ip = ip_game_controller->text().toStdString();
  tconfig.team_udp = team_udp->value();
  tconfig.team = team_number->value();
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
  auto sonar = sonars_[self];
  config.posX = px->value();
  config.posY = py->value();
  config.orientation = pt->value();
  config.sonar_enabled = sonar->isChecked();
  if(self == WO_TEAM_COACH) {
    config.posZ = z_coach->value();
  }
  config.team = team_number->value();
  if(self == WO_ALT1 || self == WO_ALT2)
    config.team = 89; // Set it to something unused so it's not on game controller
  config.self = MAP(self);
  config.team_broadcast_ip = ip_broadcast->text().toStdString();
  config.audio_enabled = audioEnabled->isChecked();
  config.svm_enabled = svmEnabled->isChecked();
  config.team_udp = team_udp->value();
  return config;
}


QStringList TeamConfigWindow::getUploadList() {
  QStringList list;
  if (ip_1->text()!="" && select_1->isChecked()) list.append(ip_1->text());
  if (ip_2->text()!="" && select_2->isChecked()) list.append(ip_2->text());
  if (ip_3->text()!="" && select_3->isChecked()) list.append(ip_3->text());
  if (ip_4->text()!="" && select_4->isChecked()) list.append(ip_4->text());
  if (ip_5->text()!="" && select_5->isChecked()) list.append(ip_5->text());
  if (ip_c->text()!="" && select_c->isChecked()) list.append(ip_c->text());
  if (ip_a1->text()!="" && select_a1->isChecked()) list.append(ip_a1->text());
  if (ip_a2->text()!="" && select_a2->isChecked()) list.append(ip_a2->text());
  return list;
}

void TeamConfigWindow::startNaoQi() {
  uploadStatusUpdated(UploadStatus::Waiting);
  cout << "Team Manager Window: Starting NaoQi's\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]);
    executor_.startNaoqi(ip);
  }
  executor_.notifyCompletion(status_callback);
}

void TeamConfigWindow::stopNaoQi() {
  uploadStatusUpdated(UploadStatus::Waiting);
  cout << "Team Manager Window: Stopping NaoQi's\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]);
    executor_.stopNaoqi(ip);
  }
  executor_.notifyCompletion(status_callback);
}

void TeamConfigWindow::restartInterpreter() {
  cout << "Team Manager Window: Restarting Interpreter\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]);
    UTMainWnd::inst()->sendUDPCommand(ip, ToolPacket::RestartInterpreter);
  }
}

void TeamConfigWindow::statusCallback(bool success) {
  if(success)
    uploadStatusUpdated(UploadStatus::Completed);
  else
    uploadStatusUpdated(UploadStatus::Failed);
}

void TeamConfigWindow::compileEverything() {
  uploadStatusUpdated(UploadStatus::Waiting);
  executor_.compile();
  executor_.notifyCompletion(status_callback);
}

void TeamConfigWindow::uploadEverything() {
  uploadStatusUpdated(UploadStatus::Waiting);
  uploadTime();
  uploadBinary();
  uploadInterpreter();
  uploadMotionVisionConfig();
  uploadColor();
  uploadRobotConfig();
  //uploadWireless();
  executor_.notifyCompletion(status_callback);
}

void TeamConfigWindow::uploadBinary() {

  //cout << "Team Manager Window: Upload Binary\n";
  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();
    executor_.sendBinary(ip, false, optimizeEnabled->isChecked());
    if(verifyEnabled->isChecked()) {
      executor_.verifyBinary(ip, false, optimizeEnabled->isChecked());
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
    if(verifyEnabled->isChecked()) {
      executor_.verifyConfigFiles(ip, false);
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
  //cout << "Team Manager Window: Upload Py\n";

  auto list = getUploadList();
  int size=list.size();
  for (int i=0; i<size; i++) {
    QString ip = getFullIP(list[i]); //toLatin1().data();

    executor_.sendPython(ip, false);
    if(verifyEnabled->isChecked()) {
      executor_.verifyPython(ip, false);
    }
  }
}

void TeamConfigWindow::checkStatus(){
  if (!this->isVisible())
    return;
  updateSSH();
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
    case ProcessExecutor::Connecting: label->setText("..."); break;
    case ProcessExecutor::Connected: label->setText("CE"); break;
    case ProcessExecutor::Alive: label->setText("A"); break;
    case ProcessExecutor::Dead: label->setText("X"); break;
    default: label->setText("-"); break;
  }
}

void TeamConfigWindow::updateUploadStatus(UploadStatus status) {
  switch(status) {
    case UploadStatus::NothingQueued:
      lblUploadStatus->setPixmap(QPixmap(":/images/empty.png")); break;
    case UploadStatus::Waiting: {
      QMovie *movie = new QMovie(":/images/loading.gif");
      lblUploadStatus->setMovie(movie);
      movie->start();
      break;
    }
    case UploadStatus::Completed:
      lblUploadStatus->setPixmap(QPixmap(":/images/checkmark.png")); break;
    case UploadStatus::Failed:
      lblUploadStatus->setPixmap(QPixmap(":/images/error.png")); break;
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
  QString common = ip_common->text();
  if (!common.endsWith("."))
    common += ".";
  return common + suffix;
}

void TeamConfigWindow::updateSSH() {
  for(auto robot : robots_) {
    auto box = ipboxes_[robot];
    if(box->text() == "") {
      clearRobotStatus(robot);
      continue;
    }
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
