#include <tool/UTMainWnd.h>

#include <ctime>

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
#include <tool/args/ArgumentParser.h>

#include <communications/TCPConnection.h>
#include <common/WorldObject.h>
#include <common/RobotInfo.h>
#include <communications/CommunicationModule.h>

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

#include <boost/filesystem/operations.hpp>
#include <chrono>

UTMainWnd* UTMainWnd::instance_ = NULL;

UTMainWnd::UTMainWnd(const Arguments& args) :
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
  connect (visionWnd_, SIGNAL(gotoSnapshot(int)), this, SLOT(gotoSnapshot(int)));
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
  connect (bypassVisionRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (visionOnlyRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (fullProcessRadioButton, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (onDemandCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (logStreamCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (coreBehaviorsCheckBox, SIGNAL(toggled(bool)), this, SLOT(saveConfig(bool)));
  connect (startSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  connect (endSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  connect (currentFrameSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)) );
  connect (stepSpin, SIGNAL(valueChanged(int)), this, SLOT(saveConfig(int)));
  
  annotations_ = new AnnotationGroup();
  emit annotationsUpdated(annotations_);

  rcPath_ = dataDirectory() + "/.utnaotoolrc";
  current_index_ = -1;
  runningCore_ = false;
  coreAvailable_ = false;
  isStreaming_ = false;
  loading_ = false;

  currentFrameSpin->setRange(0,0);
  frameSlider->setRange(0,0);

  loadConfig();
  if(args.open_previous) {
    setFrameBounds(args.frame_bounds[0], args.frame_bounds[1]);
  } else {
    // Setting the start/end frames only makes sense if we're reloading a
    // recently-opened log, because otherwise we probably don't know how many
    // frames are in the log we're opening.
    setFrameBounds();
  }
  if(!args.log_path.empty()) {
    auto path = boost::filesystem::canonical(args.log_path);
    config_.logPath = path.string();
  }

  if(args.open_previous) {
    if(config_.logPath.empty()) 
      printf("Can't reopen the last log because the path was "
        "not specified, and the path does not exist in your "
        "configuration file at: %s\n", rcPath_.c_str()
      );
    else reopenLog();
  }
  else if(args.open_recent) openRecent();
  else if(!config_.logPath.empty() && !args.log_path.empty())
    loadLog(config_.logPath);
  
  if(!args.ip_address.empty())
    filesWnd_->setCurrentLocation(args.ip_address);
  if(args.bypass_vision)
    bypassVisionRadioButton->setChecked(true);
  if(args.vision_only)
    visionOnlyRadioButton->setChecked(true);
  if(args.run_core)
    setCore(true);
}

void UTMainWnd::handleStreamFrame() {
  gotoSnapshot(0);
}

void UTMainWnd::processStreamBuffer(const StreamBuffer& buffer) {
  LogReader stream_reader(buffer);
  bool res = stream_reader.readMemory(stream_memory_, frame_id_++);
  if (!res) {
    std::cout << "Problem reading memory from tcp message" << std::endl;
    return;
  }
  emit newStreamFrame();
  // LogViewer the stream
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

void UTMainWnd::processStreamExit() {
  if(logger_->isOpen()) {
    annotations_->save(logger_->directory());
    annotations_->clear();
    annotationsUpdated(annotations_);
    logger_->close();
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
  config_.logPath = directory;
  saveConfig();
  printf("Loading frames %i to %i of file %s\n", config_.logStart, config_.logEnd, directory);
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
  std::string textfile = config_.logPath + "/frames.txt";
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
    if(config_.logWindowConfig.filterTextLogs)
      visionCore_->textlog()->setFilters(
        config_.logWindowConfig.minLevel,
        config_.logWindowConfig.maxLevel,
        static_cast<TextLogger::Type>(config_.logWindowConfig.module)
      );
    else visionCore_->textlog()->clearFilters();

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
  auto cache = MemoryCache::read(memory);
  bool locEnabled = cache.localization_mem != nullptr;
  visionCore_->updateMemory(&memory,config_.bypassVision);
  if(start) {
    // load either sim or robot color tables based on the first frame of the log
    if (config_.bypassVision) {
      visionCore_->interpreter_->initFromMemory();
    } else {
      visionCore_->vision_->initSpecificModule();
      visionCore_->interpreter_->restart();
    }
    if(!config_.visionOnly && locEnabled)
      visionCore_->localization_->initFromMemory();
    visionCore_->audio_->initFromMemory();
    visionCore_->enableTextLogging("core.txt");
  }
  if(config_.visionOnly || config_.fullProcess)
    visionCore_->vision_->processFrame();
  if(config_.bypassVision || config_.fullProcess){
    if(locEnabled)
      visionCore_->localization_->processFrame();
    visionCore_->opponents_->processFrame();
    if(config_.coreBehaviors)
      visionCore_->interpreter_->processBehaviorFrame();
  }
}

void UTMainWnd::runLog() {
  stopStream();
  if (runningCore_) {
    runningCore_ = false;
    loadLog(config_.logPath.c_str());
    std::string textfile = config_.logPath + "/frames.txt";
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
  QString address = getCurrentAddress();
  if(address.contains("core")) address = "127.0.0.1";
  frame_id_ = 0;
  tcpclient_ = std::make_unique<TCPClient>(CommInfo::TOOL_TCP_PORT);
  tcpclient_->register_receiver(&UTMainWnd::processStreamBuffer, this);
  tcpclient_->loop_client(address.toStdString());
  logSelectWnd_->sendLogSettings();
  emit setStreaming(true);
}

void UTMainWnd::stopStream() {
  tcpclient_.reset();
  processStreamExit();
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
  if(config_.logPath.empty()) return;
  // We're most likely opening the log that we were recently using. Often this
  // takes place after a crash, so we try to reset the state as closely as possible
  // by running core (if previously selected) and going to the frame we were viewing
  // before the tool closed.
  bool core = runCoreRadio->isChecked();
  int frame = config_.logFrame;
  loadLog(config_.logPath);
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
  printf("Opening most recent log based on modification date.\n");
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
  if(config_.loadFromFile(rcPath_)) {
    loadingConfig(config_);
  }
  onDemandCheckBox->setChecked(config_.onDemand);
  bypassVisionRadioButton->setChecked(config_.bypassVision);
  visionOnlyRadioButton->setChecked(config_.visionOnly);
  fullProcessRadioButton->setChecked(config_.fullProcess);
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
  config_.bypassVision = bypassVisionRadioButton->isChecked();
  config_.visionOnly = visionOnlyRadioButton->isChecked();
  config_.fullProcess = fullProcessRadioButton->isChecked();
  config_.logStream = logStreamCheckBox->isChecked();
  config_.coreBehaviors = coreBehaviorsCheckBox->isChecked();
  config_.logStart = startSpin->value();
  config_.logEnd = endSpin->value();
  config_.logFrame = currentFrameSpin->value();
  config_.logStep = stepSpin->value();
  savingConfig(config_);
  config_.saveToFile(rcPath_);
}

void UTMainWnd::setFrameBounds(int start, int end) {
  startSpin->setValue(start);
  endSpin->setValue(end);
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
