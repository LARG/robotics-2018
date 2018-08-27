#include <QtGui>
#include <tool/LogSelectWindow.h>
#include <tool/UTMainWnd.h>
#include <common/ToolPacket.h>
#include <iostream>
#include <fstream>

using namespace std;

unsigned int MAX_MODULES_PER_COLUMN = 15;

LogSelectWindow* LogSelectWindow::instance_ = NULL;

LogSelectWindow::LogSelectWindow(QMainWindow* pa, std::vector<std::string> &block_names) : ConfigWindow(pa), naoUDP(NULL), block_names_(block_names.begin(), block_names.end()) {
  instance_ = this;
  loadConfig();
}

void LogSelectWindow::init() {
  if(initialized_) return;
  centralWidget = new QWidget;
  this->setCentralWidget(centralWidget);
  QGridLayout *layout = new QGridLayout;
  centralWidget->setLayout(layout);

  nmodules_ = block_names_.size();

  // module names
  QString moduleNames[nmodules_];

  auto it = block_names_.begin();
  for (int i = 0; i < nmodules_; i++) {
    moduleNames[i] = QString::fromStdString(*it++);
  }

  //QLabel* batchLabel = new QLabel("Batch");

  moduleLabels = new QLabel*[nmodules_];
  moduleChecks = new QCheckBox*[nmodules_];

  // set values, add to layout
  int column = 0;
  int row = 0;
  for (int i = 0; i < nmodules_; i++) {
    moduleLabels[i] = new QLabel;
    moduleChecks[i] = new QCheckBox;
    if (row == 0) {
      QLabel* moduleLabel = new QLabel("Module");
      QLabel* checkLabel = new QLabel("Logged?");
      moduleLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
      checkLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );

      layout->addWidget(moduleLabel,row,column);
      layout->addWidget(checkLabel,row,column+1);
      row++;
    }
    moduleLabels[i]->setText(moduleNames[i]);

    if (moduleNames[i] == "vision_frame_info") {
      moduleChecks[i]->setChecked(true);
      moduleChecks[i]->setDisabled(true);
    }
    if (moduleNames[i] == "robot_state") {
      moduleChecks[i]->setChecked(true);
      moduleChecks[i]->setDisabled(true);
    }
    if (moduleNames[i] == "robot_info") {
      moduleChecks[i]->setChecked(true);
      moduleChecks[i]->setDisabled(true);
    }

    // add to layout
    layout->addWidget(moduleLabels[i], row, column);
    layout->addWidget(moduleChecks[i], row, column+1);
    connect(moduleChecks[i], SIGNAL(toggled(bool)), this, SLOT(controlsChanged()));

    row++;
    if ((uint16_t)(row - 1) == MAX_MODULES_PER_COLUMN) {
      row = 0;
      column += 2;
    }
  }

  sendButton = new QPushButton("Send");
  logButton = new QPushButton("Logging is OFF");
  forceStopLogButton = new QPushButton("Force stop logging");
  batchButton = new QCheckBox();
  frameCount = new QSpinBox();
  frameCount->setMaximum(500);
  frameCount->setValue(0);
  log_enabled_ = false;

  frequency = new QDoubleSpinBox();
  frequency->setMaximum(30.0);
  frequency->setMinimum(0.0);
  frequency->setValue(0.0);
  frequency->setSingleStep(0.5);
  QLabel* freqLabel = new QLabel("Interval");
  freqLabel->setText("Log Interval (s)");


  layout->addWidget(sendButton, nmodules_+2, 0);
  layout->addWidget(logButton, nmodules_+3, 0);
  layout->addWidget(frameCount, nmodules_+3, 1);
  layout->addWidget(forceStopLogButton, nmodules_+3, 2);
  layout->addWidget(frequency, nmodules_ + 4, 1);
  layout->addWidget(freqLabel, nmodules_ + 4, 0);

  //layout->addWidget(batchButton, nmodules_+4, 1); // no batch yet
  //layout->addWidget(batchLabel, nmodules_+4, 0);

  // Todd: add some 'group selection' buttons
  int NUM_GROUPS = 5;
  groupLabels = new QLabel*[NUM_GROUPS];
  groupChecks = new QCheckBox*[NUM_GROUPS];

  vector<string> labels = {"Localization", "Vision", "Vision w/ Raw", "Behavior", "ALL"};

  for (int i = 0; i < NUM_GROUPS; i++){
    groupLabels[i] = new QLabel;
    groupLabels[i]->setText(labels[i].c_str());
    layout->addWidget(groupLabels[i], nmodules_+5+i, 0);
    groupChecks[i] = new QCheckBox;
    layout->addWidget(groupChecks[i], nmodules_+5+i, 1);
  }

  connect (logButton, SIGNAL(clicked()), this, SLOT(toggleLogEnabled()));
  connect (forceStopLogButton, SIGNAL(clicked()), this, SLOT(logModeOff()));
  connect (sendButton, SIGNAL(clicked()), this, SLOT(sendLogSettings()));

  int ind = 0;
  connect(groupChecks[ind++], SIGNAL(toggled(bool)), this, SLOT(locGroupToggled(bool)));
  connect(groupChecks[ind++], SIGNAL(toggled(bool)), this, SLOT(visionGroupToggled(bool)));
  connect(groupChecks[ind++], SIGNAL(toggled(bool)), this, SLOT(visionRawGroupToggled(bool)));
  connect(groupChecks[ind++], SIGNAL(toggled(bool)), this, SLOT(behaviorGroupToggled(bool)));
  connect(groupChecks[ind++], SIGNAL(toggled(bool)), this, SLOT(allGroupToggled(bool)));


  resize(120,200);

  setWindowTitle(tr("Select memory blocks to stream/log"));
  initialized_ = true;

}

LogSelectWindow* LogSelectWindow::inst() { return instance_; }

void LogSelectWindow::saveConfig(ToolConfig& config) {
  config.memorySelectConfig = config_;
}

void LogSelectWindow::loadConfig(const ToolConfig& config) {
  config_ = config.memorySelectConfig;
  loadConfig();
}

void LogSelectWindow::loadConfig() {
  if(!initialized_) {
    block_names_.insert(
      config_.additional_blocks.begin(), config_.additional_blocks.end()
    );
    nmodules_ = block_names_.size();
    init();
    initialized_ = true;
  }
  for(int j = 0; j < nmodules_; j++) {
    auto label = moduleLabels[j]->text().toStdString();
    auto it = config_.selected_blocks.find(label);
    if(it != config_.selected_blocks.end()) {
      moduleChecks[j]->setChecked(it->second);
    }
  }
}

void LogSelectWindow::sendLogSettings() {
  sendLogSettings(UTMainWnd::inst()->getCurrentAddress());
}

void LogSelectWindow::sendLogSettings(QString ip){

  cout << "Send log settings" << endl;

  ToolPacket tp(ToolPacket::LogSelect);
  QString data;

  //std::vector<bool> en;
  //en.resize(nmodules_, false);

  for (int i = 0; i < nmodules_; i++){
    data += moduleLabels[i]->text();
    if (moduleChecks[i]->isChecked())
      cout << moduleLabels[i]->text().toStdString() << endl;
    if (moduleChecks[i]->isChecked())
      data += " 1";
    else
      data += " 0";
    data += ",";
  }
  data += '|';
  memcpy(tp.data.data(), data.toStdString().c_str(), ToolPacket::DATA_LENGTH);
  // objectSelect->updatePacket(tp);
  UTMainWnd::inst()->sendUDPCommand(ip, tp);

  cout << "Selections sent" << endl;

}

void LogSelectWindow::toggleLogEnabled() {
  if (log_enabled_) {
    requestStopLog();
  } else {
    logModeOn();
    ToolPacket tp(ToolPacket::LogBegin);
    tp.frames = frameCount->value();
    tp.interval = frequency->value();
    listenForLoggingStatus();
    UTMainWnd::inst()->sendUDPCommandToCurrent(tp);
  }
}

void LogSelectWindow::requestStopLog() {
  logButton->setEnabled(false);
  ToolPacket tp(ToolPacket::LogEnd);
  UTMainWnd::inst()->sendUDPCommandToCurrent(tp);
  listenForLoggingStatus();
}

void LogSelectWindow::logModeOn() {
  log_enabled_ = true;
  logButton->setText("Logging is ON");
}

void LogSelectWindow::logModeOff(bool force) {
  log_enabled_ = false;
  logButton->setEnabled(true);
  logButton->setText("Logging is OFF");
  if(force && naoUDP) {
    delete naoUDP;
    naoUDP = NULL;
  }
}

void LogSelectWindow::startMultiLogging(vector<QString> ips) {
  for(auto ip : ips) {
    sendLogSettings(ip);
    ToolPacket tp(ToolPacket::LogBegin);
    tp.frames = frameCount->value();
    tp.interval = frequency->value();
    UTMainWnd::inst()->sendUDPCommand(ip, tp);
    printf("Logging started on %s\n", ip.toStdString().c_str());
  }
}

void LogSelectWindow::stopMultiLogging(vector<QString> ips, function<void(QString)> callback) {
  if (naoUDP != NULL)
    delete naoUDP;
  naoUDP = new UDPWrapper(CommInfo::TOOL_UDP_PORT, false, "127.0.0.1", UDPWrapper::Inbound);
  auto listener = [=]() {
    set<QString> awaiting;
    for(auto ip : ips) awaiting.insert(ip);
    while(true) {
      ToolPacket tp;
      bool res = naoUDP->recv(tp);
      if(!res) continue;
      if(tp.message == ToolPacket::LogComplete) {
        std::string sip = naoUDP->senderAddress().to_string();
        auto qsip = QString::fromStdString(sip);
        awaiting.erase(qsip);
        printf("Logging completed on %s\n", sip.c_str());
        callback(qsip);
      }
      if(awaiting.size() == 0) break;
    }
  };
  naoUDP->startListenThread(listener);
}

void LogSelectWindow::listenForLoggingStatus() {
  auto address = UTMainWnd::inst()->getCurrentAddress();
  if (naoUDP != NULL)
    delete naoUDP;
  naoUDP = new UDPWrapper(CommInfo::TOOL_UDP_PORT, false, address.toStdString().c_str(), UDPWrapper::Inbound);
  naoUDP->startListenThread(&LogSelectWindow::listenUDP,this);
}

void LogSelectWindow::listenUDP() {
  ToolPacket tp;
  bool res = this->naoUDP->recv(tp);
  if(!res) return;
  if(tp.message = ToolPacket::LogComplete)
    this->logModeOff(false);
}

void LogSelectWindow::locGroupToggled(bool toggle){
  for (int i = 0; i < nmodules_; i++){
    if (moduleLabels[i]->text() == "game_state" ||
        moduleLabels[i]->text() == "localization" ||
        moduleLabels[i]->text() == "team_packets" ||
        moduleLabels[i]->text() == "vision_odometry" ||
        moduleLabels[i]->text() == "vision_joint_angles" ||
        moduleLabels[i]->text() == "world_objects" ||
        moduleLabels[i]->text() == "vision_walk_request")
    moduleChecks[i]->setChecked(toggle);
  }
}

void LogSelectWindow::visionGroupToggled(bool toggle){
  for (int i = 0; i < nmodules_; i++){
    if (moduleLabels[i]->text() == "robot_vision" ||
        moduleLabels[i]->text() == "camera_info" ||
        moduleLabels[i]->text() == "world_objects" ||
        moduleLabels[i]->text() == "vision_body_model" ||
        moduleLabels[i]->text() == "vision_joint_angles" ||
        moduleLabels[i]->text() == "vision_sensors" ||
        moduleLabels[i]->text() == "game_state"
        )
    moduleChecks[i]->setChecked(toggle);
  }
}

void LogSelectWindow::visionRawGroupToggled(bool toggle){
  for (int i = 0; i < nmodules_; i++){
    if (moduleLabels[i]->text() == "robot_vision" ||
        moduleLabels[i]->text() == "camera_info" ||
        moduleLabels[i]->text() == "world_objects" ||
        moduleLabels[i]->text() == "vision_body_model" ||
        moduleLabels[i]->text() == "vision_joint_angles" ||
        moduleLabels[i]->text() == "vision_sensors" ||
        moduleLabels[i]->text() == "raw_image" ||
        moduleLabels[i]->text() == "game_state"
        )
    moduleChecks[i]->setChecked(toggle);
  }
}

void LogSelectWindow::behaviorGroupToggled(bool toggle){
  for (int i = 0; i < nmodules_; i++){
    if (moduleLabels[i]->text() == "game_state" ||
        moduleLabels[i]->text() == "localization" ||
        //moduleLabels[i]->text() == "opponents" ||
        //moduleLabels[i]->text() == "robot_vision" ||
        moduleLabels[i]->text() == "team_packets" ||
        moduleLabels[i]->text() == "vision_odometry" ||
        moduleLabels[i]->text() == "vision_joint_angles" ||
        moduleLabels[i]->text() == "behavior" ||
        moduleLabels[i]->text() == "vision_walk_request" ||
        moduleLabels[i]->text() == "world_objects")
    moduleChecks[i]->setChecked(toggle);
  }
}

void LogSelectWindow::allGroupToggled(bool toggle){
  for (int i = 0; i < nmodules_; i++){
    if (moduleLabels[i]->text() == "vision_frame_info") continue;
    if (moduleLabels[i]->text() == "robot_state") continue;
    moduleChecks[i]->setChecked(toggle);
  }
}

void LogSelectWindow::updateSelectedIP(QString address){
  sendButton->setText("Send to "+address);
}

void LogSelectWindow::controlsChanged() {
  if(loading_) return;
  for(int i = 0; i < nmodules_; i++) {
    auto label = moduleLabels[i]->text().toStdString();
    config_.selected_blocks[label] = moduleChecks[i]->isChecked();
  }
  ConfigWindow::saveConfig();
}

