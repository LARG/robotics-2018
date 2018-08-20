#include <tool/UTMainWnd.h>

#include <ctime>

#include <thread>

// windows
#include <tool/FilesWindow.h>
#include <tool/LogEditorWindow.h>
#include <tool/LogSelectWindow.h>
#include <tool/LogWindow.h>
#include <tool/MotionWindow.h>
#include <tool/PlotWindow.h>
#include <tool/VisionWindow.h>
#include <tool/WorldWindow.h>
#include <tool/JointsWindow.h>
#include <tool/WalkWindow.h>
#include <tool/StateWindow.h>
#include <tool/CameraWindow.h>
#include <tool/SensorWindow.h>
#include <tool/TeamConfigWindow.h>
#include <common/annotations/AnnotationGroup.h>

//Networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <common/WorldObject.h>
#include <common/RobotInfo.h>

#include <memory/PrivateMemory.h>
#include <memory/LogReader.h>
#include <memory/MemoryFrame.h>
#include <memory/MemoryCache.h>

// memory blocks
#include <memory/BodyModelBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/ImageBlock.h>
#include <memory/SensorBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SimEffectorBlock.h>

#include <common/RobotConfig.h>
#include <common/Calibration.h>

#include <audio/AudioModule.h>
#include <localization/LocalizationModule.h>
#include <opponents/OppModule.h>
#include <InterpreterModule.h>

#include <math/Pose3D.h>
#include <math/RotationMatrix.h>
#include <math/Vector3.h>

UTMainWnd* UTMainWnd::instance_ = NULL;
bool UTMainWnd::reload_ = false;

UTMainWnd::UTMainWnd(const char *directory, bool core):
//  memory_log_(NULL),
  stream_memory_(false,MemoryOwner::TOOL_MEM, 0, 1),
  filesWnd_(NULL),
  logEditorWnd_(NULL),
  logSelectWnd_(NULL),
  logWnd_(NULL),
  motionWnd_(NULL),
  plotWnd_(NULL),
  visionWnd_(NULL),
  worldWnd_(NULL),
  jointsWnd_(NULL),
  walkWnd_(NULL),
  stateWnd_(NULL),
  cameraWnd_(NULL),
  sensorWnd_(NULL),
  teamWnd_(NULL)
{
  instance_ = this;
  srand(0);

  setupUi(this);

  memory_ = NULL;
  logWnd_ = new LogWindow(this);
  motionWnd_ = new MotionWindow(this);
  plotWnd_ = new PlotWindow();
  worldWnd_ = new WorldWindow(this);
  jointsWnd_ = new JointsWindow();
  walkWnd_ = new WalkWindow();
  stateWnd_ = new StateWindow();
  cameraWnd_ = new CameraWindow(this);
  sensorWnd_ = new SensorWindow();
  teamWnd_ = new TeamConfigWindow(this);

  std::vector<std::string> block_names;
  visionCore_ = new VisionCore(CORE_TOOL,false,0,1);
  visionWnd_ = new VisionWindow(this, visionCore_);
  visionCore_->memory_->getBlockNames(block_names,false);
  logEditorWnd_ = new LogEditorWindow(this);
  logSelectWnd_ = new LogSelectWindow(this,block_names);
  filesWnd_ = new FilesWindow(this);

  logger_ = new FileLogWriter();
  logger_->setType(CORE_TOOL);

  connect (this, SIGNAL(newStreamFrame()), this, SLOT(handleStreamFrame()));

  connect (frameSlider, SIGNAL(valueChanged(int)), this, SLOT(gotoSnapshot(int)) );
  connect (currentFrameSpin, SIGNAL(valueChanged(int)), this, SLOT(gotoSnapshot(int)) );
  connect (stepSpin, SIGNAL(valueChanged(int)), this, SLOT(setFrameStep(int)));

  connect (jointsButton, SIGNAL(clicked()), this, SLOT(openJointsWnd()) );
  connect (walkButton, SIGNAL(clicked()), this, SLOT(openWalkWnd()));
  connect (sensorButton, SIGNAL(clicked()), this, SLOT(openSensorWnd()) );
  connect (motionButton, SIGNAL(clicked()), this, SLOT(openMotionWnd()) );
  connect (plotButton, SIGNAL(clicked()), this, SLOT(openPlotWnd()) );
  connect (filesButton, SIGNAL(clicked()), this, SLOT(openFilesWnd()) );
  connect (fullVisionButton, SIGNAL(clicked()), this, SLOT(openVisionWnd()) );
  connect (worldButton, SIGNAL(clicked()), this, SLOT(openWorldWnd()) );
  connect (logButton, SIGNAL(clicked()), this, SLOT(openLogWnd()) );
  connect (logEditorButton, SIGNAL(clicked()), this, SLOT(openLogEditorWnd()) );
  connect (logSelectButton, SIGNAL(clicked()), this, SLOT(openLogSelectWnd()) );
  connect (stateButton, SIGNAL(clicked()), this, SLOT(openStateWnd()) );
  connect (cameraButton, SIGNAL(clicked()), this, SLOT(openCameraWnd()) );
  connect (teamButton, SIGNAL(clicked()), this, SLOT(openTeamWnd()) );

  connect (motionWnd_->motion_, SIGNAL(prevSnapshot()), this, SLOT(prevSnapshot()) );
  connect (motionWnd_->motion_, SIGNAL(nextSnapshot()), this, SLOT(nextSnapshot()) );

  connect (worldWnd_, SIGNAL(prevSnapshot()), this, SLOT(prevSnapshot()) );
  connect (worldWnd_, SIGNAL(nextSnapshot()), this, SLOT(nextSnapshot()) );

  connect (logWnd_, SIGNAL(prevSnapshot()), this, SLOT(prevSnapshot()) );
  connect (logWnd_, SIGNAL(nextSnapshot()), this, SLOT(nextSnapshot()) );

  connect (plotWnd_, SIGNAL(gotoSnapshot(int)), this, SLOT(gotoSnapshot(int)) );
  connect (plotWnd_, SIGNAL(prevSnapshot()), this, SLOT(prevSnapshot()) );
  connect (plotWnd_, SIGNAL(nextSnapshot()), this, SLOT(nextSnapshot()) );

  connect (visionWnd_, SIGNAL(prevSnapshot()), this, SLOT(prevSnapshot()) );
  connect (visionWnd_, SIGNAL(nextSnapshot()), this, SLOT(nextSnapshot()) );
  connect (visionWnd_, SIGNAL(setCore(bool)), this, SLOT(setCore(bool)) );
  connect (this, SIGNAL(setStreaming(bool)), visionWnd_, SLOT(setStreaming(bool)) );
  connect (this, SIGNAL(setStreaming(bool)), worldWnd_, SLOT(handleStreaming(bool)) );
  connect (this, SIGNAL(newLogFrame(int)), visionWnd_, SLOT(handleNewLogFrame(int)));
  connect (this, SIGNAL(newLogLoaded(LogViewer*)), visionWnd_, SLOT(handleNewLogLoaded(LogViewer*)));
  connect (this, SIGNAL(newStreamFrame()), visionWnd_, SLOT(handleNewStreamFrame()));
  connect (this, SIGNAL(runningCore()), visionWnd_, SLOT(handleRunningCore()));
  connect (this, SIGNAL(annotationsUpdated(AnnotationGroup*)), worldWnd_, SLOT(updateAnnotations(AnnotationGroup*)));

  connect (actionOpen_Log, SIGNAL(triggered()), this, SLOT(openLog()) );
  connect (actionReOpen_Log, SIGNAL(triggered()), this, SLOT(reopenLog()) );
  connect (actionOpen_Recent_Log, SIGNAL(triggered()), this, SLOT(openRecent()));
  connect (actionReRun_Core, SIGNAL(triggered()), this, SLOT(rerunCore()));

  connect (runCoreRadio, SIGNAL(clicked()), this, SLOT(runCore()) );
  connect (viewLogRadio, SIGNAL(clicked()), this, SLOT(runLog()) );
  connect (streamRadio, SIGNAL(clicked()), this, SLOT(runStream()));
  connect (streamRadio, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (localizationOnlyRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (visionOnlyRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (locVisRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (onDemandCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (logStreamCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (coreBehaviorsCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (startSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  connect (endSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  connect (currentFrameSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)) );
  connect (stepSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  
  annotations_ = new AnnotationGroup();
  emit annotationsUpdated(annotations_);

  rcPath = dataDirectory() + "/.utnaotoolrc";
  current_index_ = -1;
  runningCore_ = false;
  coreAvailable_ = false;
  isStreaming_ = false;
  loading_ = false;

  currentFrameSpin->setRange(0,0);
  frameSlider->setRange(0,0);

  new std::thread(server,this);

  loadConfig();
  if(!reload_) { // Unless we're reloading the latest log, don't set start/end up front
    setFrameRange();
  }
  if(directory) config_.logFile = directory;
  if(reload_) reopenLog();
  else if(directory) {
    loadLog(config_.logFile);
    if (core) {
      runningCore_ = true;
      runCore();
    }
  }
}

void UTMainWnd::handleStreamFrame() {
  gotoSnapshot(0);
}

void server(UTMainWnd *main) {
  try {
    while(true) {
      while(!main->isVisible() || !main->isStreaming())
        sleep(0.01);
      boost::asio::io_service io_service;
      socket_ptr sock(new tcp::socket(io_service));
      tcp::acceptor acceptor(io_service,tcp::endpoint(tcp::v4(),CommInfo::TOOL_TCP_PORT));
      acceptor.accept(*sock);
      main->processStream(sock);
    }
  } catch (...) {
    fprintf(stderr, "Error starting TCP server - may have multiple tool instances running.\n");
  }
}

void UTMainWnd::processStream(socket_ptr sock) {
  std::size_t ret;
  unsigned long &send_len = stream_msg_.send_len_;
  std::size_t expected_len = sizeof(send_len);
  try {
    for (;;) {
      ret = boost::asio::read(*sock,boost::asio::buffer(&send_len,expected_len));
      if (ret != expected_len) {
        std::cout << "Couldn't read send_len " << ret << " " << expected_len << std::endl << std::flush;
        return;
      }
      if (send_len > MAX_STREAMING_MESSAGE_LEN) {
        std::cout << "MESSAGE TOO LARGE " << send_len << " " << MAX_STREAMING_MESSAGE_LEN << std::endl << std::flush;
        return;
      }
      ret = boost::asio::read(*sock,boost::asio::buffer(&(stream_msg_.orig_len_),expected_len));
      if (ret != expected_len) {
        std::cout << "Couldn't read orig_len " << ret << " " << expected_len << std::endl << std::flush;
        return;
      }
      ret = boost::asio::read(*sock,boost::asio::buffer(&stream_msg_.data_,send_len - 2 * expected_len));
      char *msg = (char*)stream_msg_.postReceive(ret);
      if (msg == NULL) {
        std::cout << "Invalid tcp message" << std::endl << std::flush;
        return;
      }
      StreamBuffer sb(msg, stream_msg_.orig_len_);
      LogReader stream_reader(sb);
      bool res = stream_reader.readMemory(stream_memory_, current_index_);
      if (!res) {
        std::cout << "Problem reading memory from tcp message" << std::endl;
        return;
      }
      delete []msg;
      emit newStreamFrame();

      // Log the stream
      if(config_.logStream) {
        if(!logger_->isOpen()) {
          logger_->open("stream", true);
          annotations_->clear();
          annotationsUpdated(annotations_);
        }
        logger_->writeMemory(stream_memory_);
        annotations_->save(logger_->directory());
      } else if(logger_->isOpen()) logger_->close();
      
    }
  } catch (boost::system::system_error) {
    if(logger_->isOpen()) {
      annotations_->save(logger_->directory());
      annotations_->clear();
      annotationsUpdated(annotations_);
      logger_->close();
    }
    std::cout << "Error reading from tcp, disconnecting" << std::endl << std::flush;
    return;
  }
}

UTMainWnd::~UTMainWnd() {
  delete logger_;
  delete visionCore_;
}

QString UTMainWnd::getCurrentAddress() {
  if (filesWnd_ == NULL) return "";
  return filesWnd_->locationBox->currentText();
}

#include <memory/WalkRequestBlock.h>

void UTMainWnd::loadLog(std::string directory) {
  loadLog(directory.c_str());
}

void UTMainWnd::loadLog(const char *directory) {
  config_.logFile = directory;
  saveConfig();
  printf("Loading frames %i to %i of file %s\n", config_.logStart, config_.logEnd, directory);
  //if(memory_log_) delete memory_log_;

  if(config_.onDemand)
    memory_log_ = std::make_unique<LogViewer>(directory, config_.logStart, config_.logEnd);
  else
    memory_log_ = std::make_unique<CachedLogViewer>(directory, config_.logStart, config_.logEnd);

  int size = memory_log_->size();
  std::cout << "Loaded " << size << " memory frames\n" << std::flush;
  if (size == 0)
    return;

  // load the corresponding text file as well
  setCore(false);
  std::string textfile = config_.logFile + "/frames.txt";
  logWnd_->loadTextFile(textfile.c_str()); // done in setCore // not always apparently

  numFrameEdit->setText(QString::number(size-1));
  currentFrameSpin->setRange(0,size-1);
  frameSlider->setRange(0,size-1);

  // update plot window with whole log if we're using it
  if (plotWnd_->isVisible()){
    plotWnd_->setMemoryLog(memory_log_.get());
  }

  int index = current_index_ >= 0 ? current_index_ : 0;
  current_index_ = -1;

  coreAvailable_ = false;
  if (runningCore_) {
    runCore();
  }

  gotoSnapshot(index);
  emit newLogLoaded(memory_log_.get());
  annotations_->load(memory_log_.get());
  annotationsUpdated(annotations_);
}


void UTMainWnd::rerunCore() {
  coreAvailable_ = false;
  runningCore_ = false;
  runCore();
}


void UTMainWnd::runCore() {
  stopStream();
  // Check if we have more than one log
  if (memory_log_ == nullptr || memory_log_->size() == 0) {
    std::cout << "No log available to run core on" << std::endl;
    return;
  }

  // Run core if core is not available
  if (!coreAvailable_) {

    // Remove any previous log that was there
    remove("core.txt");

    visionCore_->opponents_->reInit();

    for (unsigned i = 0; i < memory_log_->size(); i++) {
      runCoreFrame(i, i == 0, i == memory_log_->size() - 1);
    }
    visionCore_->disableTextLogging();
    coreAvailable_ = true;
  }

  if (!runningCore_) {
    runningCore_ = true;
    emit runningCore();
    logWnd_->loadTextFile("core.txt");
    int index = current_index_;
    current_index_ = -1;
    int size = memory_log_->size();
    numFrameEdit->setText(QString::number(size-1));
    currentFrameSpin->setRange(0,size-1);
    frameSlider->setRange(0,size-1);
    gotoSnapshot(index);
  }
}

void UTMainWnd::runCoreFrame(int i, bool start, bool end) {
  MemoryFrame& memory = memory_log_->getFrame(i);
  visionCore_->updateMemory(&memory,config_.locOnly);
  if (start) {
    // load either sim or robot color tables based on the first frame of the log
    if (config_.locOnly) {
      visionCore_->interpreter_->initFromMemory();
    } else {
      visionCore_->vision_->loadColorTables();
      visionCore_->interpreter_->restart();
    }
    visionCore_->localization_->initFromMemory();
    visionCore_->audio_->initFromMemory();
    visionCore_->enableTextLogging("core.txt");
  }
  if (config_.locOnly){
    visionCore_->localization_->processFrame();
    visionCore_->opponents_->processFrame();
    if(config_.coreBehaviors)
      visionCore_->interpreter_->processBehaviorFrame();
  } else if (config_.visOnly) {
    visionCore_->vision_->processFrame();
  } else {
    visionCore_->processVisionFrame();
  }
  visionCore_->audio_->processFrame();
}

void UTMainWnd::runLog() {
  stopStream();
  if (runningCore_) {
    runningCore_ = false;
    loadLog(config_.logFile.c_str());
    std::string textfile = config_.logFile + "/frames.txt";
    logWnd_->loadTextFile(textfile.c_str());
    int index = current_index_;
    current_index_ = -1;
    int size = memory_log_->size();
    numFrameEdit->setText(QString::number(size-1));
    currentFrameSpin->setRange(0,size-1);
    frameSlider->setRange(0,size-1);
    gotoSnapshot(index);
  }
}

void UTMainWnd::setCore(bool value) {
  if (value) {
    this->runCoreRadio->setChecked(true);
    runCore();
  }
  else {
    this->viewLogRadio->setChecked(true);
    runLog();
  }
}

void UTMainWnd::runStream() {
  runningCore_ = false;
  isStreaming_ = true;
  stream_memory_ = MemoryFrame(false,MemoryOwner::TOOL_MEM, 0, 1);
  sleep(0.05); // this is terrible
  sendUDPCommandToCurrent(ToolPacket::StreamBegin);
  logSelectWnd_->sendLogSettings();
  emit setStreaming(true);
}

void UTMainWnd::stopStream() {
  if (isStreaming_)
    sendUDPCommandToCurrent(ToolPacket::StreamEnd);
  isStreaming_ = false;
  emit setStreaming(false);
}

void UTMainWnd::gotoSnapshot(int index) {
  // do nothing if we're already at this snapshot
  if (!isStreaming_ && (index == current_index_))
    return;
  if(runningCore_ && config_.onDemand) runCoreFrame(index);
  emit newLogFrame(index);

  current_index_= index;
  currentFrameSpin->setValue(current_index_);
  frameSlider->setValue(current_index_);

  if (isStreaming_) {
    memory_ = &stream_memory_;
  } else{
    if (current_index_ >= memory_log_->size())
      current_index_ = memory_log_->size() - 1;
    memory_ = &memory_log_->getFrame(current_index_);
  }

  if (motionWnd_->isVisible())
    motionWnd_->updateMemory(memory_);
  if (plotWnd_->isVisible())
    plotWnd_->update(memory_);
  if (visionWnd_->isVisible())
    visionWnd_->update(memory_);
  if (worldWnd_->isVisible())
    worldWnd_->updateMemory(memory_);
  if (logWnd_->isVisible())
    logWnd_->updateFrame(memory_);
  if (jointsWnd_->isVisible())
    jointsWnd_->update(memory_);
  if (walkWnd_->isVisible())
    walkWnd_->update(memory_);
  if (stateWnd_->isVisible())
    stateWnd_->update(memory_);
  if (cameraWnd_->isVisible())
    cameraWnd_->update(memory_);
  if (sensorWnd_->isVisible())
    sensorWnd_->update(memory_);
}

void UTMainWnd::prevSnapshot() {
  int index = current_index_ - 1;
  if (index < 0) index=0;
  gotoSnapshot(index);
}

void UTMainWnd::nextSnapshot() {
  if (isStreaming_) return;
  unsigned int index = current_index_ + 1;
  if (!memory_log_) return;
  if (index > memory_log_->size()-1) index=memory_log_->size()-1;
  gotoSnapshot(index);
}

void UTMainWnd::reopenLog() {
  bool core = runCoreRadio->isChecked();
  int frame = config_.logFrame;
  loadLog(config_.logFile);
  currentFrameSpin->setValue(frame);
  if(core) {
    coreAvailable_ = false;
    runningCore_ = false;
    runCore();
  }
}

bool UTMainWnd::openLog() {
  QString fileName = QFileDialog::getExistingDirectory(
    this, 
    tr("Open Log File"),
    QString(getenv("NAO_HOME")) + "/logs/",
    QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly
  );
  if (fileName == NULL)
    return false;
  loadLog(fileName.toLatin1());
  return true;
}

bool UTMainWnd::openRecent() {

  // find most recent log
  QDir* logDir = new QDir(QString(getenv("NAO_HOME")) + "/logs/");
  QStringList filters;
  logDir->setSorting(QDir::Time);
  logDir->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  QFileInfoList files = logDir->entryInfoList();
  if (files.size() == 0){
    cout << "No logs" << endl;
    return false;
  }
  QFileInfo file = files.first();
  QString directory = file.filePath();
  loadLog(directory.toLatin1());
  return true;
}


void UTMainWnd::openFilesWnd() {
  filesWnd_->show();
  filesWnd_->raise();
  filesWnd_->activateWindow();
}

void UTMainWnd::openTeamWnd() {
  teamWnd_->show();
  teamWnd_->raise();
  teamWnd_->activateWindow();
}

void UTMainWnd::openLogSelectWnd() {
  logSelectWnd_->show();
  logSelectWnd_->raise();
  logSelectWnd_->activateWindow();
}

void UTMainWnd::openLogEditorWnd() {
  logEditorWnd_->show();
  logEditorWnd_->raise();
  logEditorWnd_->activateWindow();
}

void UTMainWnd::openLogWnd() {
  logWnd_->show();
  logWnd_->raise();
  logWnd_->activateWindow();
  if (memory_) logWnd_->updateFrame(memory_);
}

void UTMainWnd::openMotionWnd() {
  motionWnd_->show();
  motionWnd_->raise();
  motionWnd_->activateWindow();
  if (memory_) motionWnd_->updateMemory(memory_);
}

void UTMainWnd::openPlotWnd() {
  plotWnd_->setMemoryLog(memory_log_.get());
  plotWnd_->show();
  plotWnd_->activateWindow();
  if (memory_) plotWnd_->update(memory_);
}

void UTMainWnd::openVisionWnd() {
  visionWnd_->show();
  visionWnd_->raise();
  visionWnd_->activateWindow();
  if (memory_) visionWnd_->update(memory_);
}

void UTMainWnd::openWorldWnd() {
  worldWnd_->show();
  worldWnd_->raise();
  worldWnd_->activateWindow();
  if (memory_) worldWnd_->updateMemory(memory_);
}

void UTMainWnd::openJointsWnd() {
  jointsWnd_->show();
  jointsWnd_->raise();
  jointsWnd_->activateWindow();
  if (memory_) jointsWnd_->update(memory_);
}

void UTMainWnd::openWalkWnd() {
  walkWnd_->show();
  walkWnd_->raise();
  walkWnd_->activateWindow();
  if (memory_) walkWnd_->update(memory_);
}

void UTMainWnd::openSensorWnd() {
  sensorWnd_->show();
  sensorWnd_->raise();
  sensorWnd_->activateWindow();
  if (memory_) sensorWnd_->update(memory_);
}

void UTMainWnd::openStateWnd() {
  stateWnd_->show();
  stateWnd_->raise();
  stateWnd_->activateWindow();
  if (memory_) stateWnd_->update(memory_);
}

void UTMainWnd::openCameraWnd() {
  cameraWnd_->show();
  cameraWnd_->raise();
  cameraWnd_->activateWindow();
  if (memory_) cameraWnd_->update(memory_);
}
void UTMainWnd::remoteRestartInterpreter() {
  sendUDPCommandToCurrent(ToolPacket::RestartInterpreter);
}

void UTMainWnd::updateConfigFile() {

  if (getCurrentAddress() == "localhost") { // Not required for simulator
    cout << "Error: config file is not supported for simulator!" << endl;
    return;
  }

  RobotConfig config;
  config.robot_id = getCurrentAddress().section('.', 3).toInt();
  config.team = filesWnd_->teamNumBox->value();
  config.role = filesWnd_->roleBox->value();

  if (config.team != 23){
    cout << "WARNING! Setting team # to " << config.team << " while our RC2011 team number is 23" << endl;
  }

  // Write file to set values
  config.saveToFile("./config.txt");

  // Send the file to the robot
  QString cmd = "scp ";
  cmd += "./config.txt ";
  cmd += "nao@" + getCurrentAddress() + ":~/data/config.txt ";
  std::cout << "Executing command: " << cmd.toStdString() << std::endl;
  QProcess processSend;
  processSend.start(cmd);
  if (!processSend.waitForStarted()) {
    cout << "Error: unable to copy config.txt to " << getCurrentAddress().toStdString() << endl << flush;
    return;
  }
  processSend.waitForFinished();
  std::cout << "Done!" << std::endl;

}

void UTMainWnd::updateCalibrationFile() {

  if (getCurrentAddress() == "localhost") { // Not required for simulator
    cout << "Error: config file is not supported for simulator!" << endl;
    return;
  }

  // Send the file to the robot
  filesWnd_->sendFile("./calibration.txt","~/data","calibration.txt");
  filesWnd_->sendFile(filesWnd_->dataPath + "defaultcamera.cal","~/data","defaultcamera.cal");
  QString id = getCurrentAddress().split(".")[3];
  filesWnd_->sendFile(filesWnd_->dataPath + id + "camera.cal","~/data",id+"camera.cal");
}

void UTMainWnd::readCalibrationFile() {
  // Copy over the file from the robot first to not overwrite useful values
  QString cmd = "scp ";
  cmd += "nao@" + getCurrentAddress() + ":~/data/calibration.txt ";
  cmd += "./calibration.txt";
  std::cout << "Executing command: " << cmd.toStdString() << std::endl;
  QProcess processGet;
  processGet.start(cmd);
  if (!processGet.waitForStarted()) {
    cout << "Error: unable to copy calibration.txt from " << getCurrentAddress().toStdString() << endl << flush;
  }
  processGet.waitForFinished();

  // Read file to get values
  Calibration calibration;
  if (calibration.readFromFile("./calibration.txt")) {
    //visionWnd_->updateCalibration(calibration.tilt_top_cam_, calibration.roll_top_cam_, calibration.tilt_bottom_cam_, calibration.roll_bottom_cam_, calibration.head_pan_offset_, calibration.head_tilt_offset_);
  } else {
    cout << "Error: unable to read calibration.txt from " << getCurrentAddress().toStdString() << endl << flush;
  }
}


void UTMainWnd::sendUDPCommand(QString address, ToolPacket packet) {
  if(address == "core") return;
  UDPWrapper sender(CommInfo::TOOL_UDP_PORT, false, address.toStdString(), UDPWrapper::Outbound);
  sender.send(packet);
}

void UTMainWnd::sendUDPCommandToCurrent(ToolPacket packet) {
  QString address = getCurrentAddress();
  sendUDPCommand(address,packet);
}
  
void UTMainWnd::loadConfig() {
  loading_ = true;
  if(config_.loadFromFile(rcPath)) {
    loadingConfig(config_);
  }
  onDemandCheckBox->setChecked(config_.onDemand);
  localizationOnlyRadioButton->setChecked(config_.locOnly);
  visionOnlyRadioButton->setChecked(config_.visOnly);
  locVisRadioButton->setChecked(config_.locAndVis);
  logStreamCheckBox->setChecked(config_.logStream);
  coreBehaviorsCheckBox->setChecked(config_.coreBehaviors);
  startSpin->setValue(config_.logStart);
  endSpin->setValue(config_.logEnd);
  stepSpin->setValue(config_.logStep);
  currentFrameSpin->setSingleStep(config_.logStep);
  loading_ = false;
}

void UTMainWnd::saveConfig(double) {
  saveConfig();
}

void UTMainWnd::saveConfig(int) {
  saveConfig();
}

void UTMainWnd::saveConfig(bool) {
  saveConfig();
}

void UTMainWnd::saveConfig() {
  if(loading_) return;
  config_.streaming = streamRadio->isChecked();
  config_.onDemand = onDemandCheckBox->isChecked();
  config_.locOnly = localizationOnlyRadioButton->isChecked();
  config_.visOnly = visionOnlyRadioButton->isChecked();
  config_.locAndVis = locVisRadioButton->isChecked();
  config_.logStream = logStreamCheckBox->isChecked();
  config_.coreBehaviors = coreBehaviorsCheckBox->isChecked();
  config_.logStart = startSpin->value();
  config_.logEnd = endSpin->value();
  config_.logFrame = currentFrameSpin->value();
  config_.logStep = stepSpin->value();
  savingConfig(config_);
  config_.saveToFile(rcPath);
}

int UTMainWnd::loadInt(QTextStream &t, bool &ok) {
  QString line = t.readLine();
  if (line.isNull()) {
    ok = false;
    return 0;
  }
  return line.toInt(&ok);
}

void UTMainWnd::addressChanged() {
  if(config_.streaming) {
    streamRadio->setChecked(true);
    runStream();
  }
}

void UTMainWnd::setFrameStep(int v) {
  currentFrameSpin->setSingleStep(v);
}
