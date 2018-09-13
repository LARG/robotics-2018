#include <tool/WorldWindow.h>
#include <common/annotations/AnnotationGroup.h>
#include <tool/simulation/BehaviorSimulation.h>
#include <tool/simulation/LocalizationSimulation.h>
#include <tool/simulation/IsolatedBehaviorSimulation.h>
#include <tool/simulation/GoalieSimulation.h>
#include <communications/CommunicationModule.cpp>
#include <common/Util.h>

#define CONFIG_PATH(mode) util::format("%s/%s.yaml", util::cfgpath(util::SimViewConfigs), getName(static_cast<WorldMode>(mode))) 
using namespace std;

WorldWindow::WorldWindow(QMainWindow* pa) : ConfigWindow(pa) {
  simulation_ = nullptr;
  livecore_ = nullptr;
  annotations_ = nullptr;
  updating_ = false;
  setupUi(this);
  world->loadState((char*)"worldView.xml");
  teamColorBox->addItem("Blue");
  teamColorBox->addItem("Red");
  for(int i = 0; i < NUM_WorldModes; i++) {
    auto mode = (WorldMode)i;
    modes_.push_back((WorldMode)i);
    modeBox->addItem(getName(mode));
  }
  for(int i = 0; i < GLDrawer::NUM_DisplayOptions; i++) {
    auto option = (GLDrawer::DisplayOption)i;
    auto box = new QCheckBox(this);
    displayOptionLayout->addWidget(box);
    box->setText(GLDrawer::getName(option));
    displayBoxes_[option] = box;
    connect(box, SIGNAL(toggled(bool)), this, SLOT(displayOptionsChanged(bool)));
  }
  connect(modeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged(int)));
  connect(playButton, SIGNAL(pressed()), this, SLOT(play()));
  connect(pauseButton, SIGNAL(pressed()), this, SLOT(pause()));
  connect(restartButton, SIGNAL(pressed()), this, SLOT(restart()));
  connect(forwardButton, SIGNAL(pressed()), this, SLOT(forward()));
  connect(speedBox, SIGNAL(valueChanged(double)), this, SLOT(controlsChanged(double)));
  connect(playersBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
  connect(autoplayBox, SIGNAL(toggled(bool)), this, SLOT(controlsChanged(bool)));
  connect(teamNumberBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
  connect(teamColorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged(int)));
  connect(skipButton, SIGNAL(pressed()), this, SLOT(skip()));
  connect(skipBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
  connect(seedBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
  connect(UTMainWnd::inst(), SIGNAL(newLogLoaded(LogViewer*)), this, SLOT(restart(LogViewer*)));
  connect(world, SIGNAL(clicked(Point2D,Qt::MouseButton)), this, SLOT(fieldClicked(Point2D,Qt::MouseButton)));
  
  connect(world, SIGNAL(moved(Point2D)), simControl, SLOT(fieldHovered(Point2D)));
  connect(world, SIGNAL(groundDragged(Point2D,Point2D,Qt::MouseButton)), simControl, SLOT(fieldDragged(Point2D,Point2D,Qt::MouseButton)));
  connect(world, SIGNAL(clicked(Point2D,Qt::MouseButton)), simControl, SLOT(fieldClicked(Point2D,Qt::MouseButton)));
  
  connect(world, SIGNAL(moved(Point2D)), logWidget, SLOT(fieldHovered(Point2D)));
  connect(world, SIGNAL(groundDragged(Point2D,Point2D,Qt::MouseButton)), logWidget, SLOT(fieldDragged(Point2D,Point2D,Qt::MouseButton)));
  connect(world, SIGNAL(clicked(Point2D,Qt::MouseButton)), logWidget, SLOT(fieldClicked(Point2D,Qt::MouseButton)));
  connect(this, SIGNAL(frameLoaded(int)), logWidget, SLOT(frameLoaded(int)));

  connect(this, SIGNAL(annotationsUpdated(AnnotationGroup*)), logWidget, SLOT(updateAnnotations(AnnotationGroup*)));
  connect(this, SIGNAL(annotationsUpdated(AnnotationGroup*)), world, SLOT(updateAnnotations(AnnotationGroup*)));
  
  play_ = false; 
  wconfig_.playSpeed = 50;
  state_timer_ = new QTimer(this);
  connect(state_timer_, SIGNAL(timeout()), this, SLOT(updateSimulationState()));
  state_timer_->start();
  view_timer_ = new QTimer(this);
  connect(view_timer_, SIGNAL(timeout()), this, SLOT(updateSimulationView()));
  view_timer_->start();
 
  setMode(NoDraw);
  simControl->setWorld(world);
  logWidget->setWorld(world);
}

void WorldWindow::updateAnnotations(AnnotationGroup* annotations) {
  annotations_ = annotations;
  emit annotationsUpdated(annotations_);
}

void WorldWindow::updateDisplay(bool) {
  updating_ = true;
  for(int i = 0; i < GLDrawer::NUM_DisplayOptions; i++) {
    auto option = (GLDrawer::DisplayOption)i;
    auto box = displayBoxes_[option];
    bool checked = simconfig_.options[option];
    box->setChecked(checked);
  }
  speedBox->setValue(wconfig_.playSpeed);
  modeBox->setCurrentIndex(wconfig_.mode);
  autoplayBox->setChecked(wconfig_.autoplay);
  teamNumberBox->setValue(wconfig_.teamNumber);
  playersBox->setValue(wconfig_.simPlayers);
  teamColorBox->setCurrentIndex(wconfig_.teamColor);
  skipBox->setValue(wconfig_.skip);
  seedBox->setValue(wconfig_.seed);
  world->updateDisplay(simconfig_.options);
  updating_ = false;
}

void WorldWindow::updateMemory(MemoryFrame* mem) {
  MemoryCache cache(mem);
  updateMemory(cache);
}

void WorldWindow::updateMemory(MemoryCache cache) {
  world->updateMemory(cache.memory);
  teamPackets->updateMemory(cache);
  if(!cache.frame_info || !cache.robot_state || !cache.game_state) return;
  frames->setText(QString::number(cache.frame_info->frame_id));
  int red = cache.robot_state->team_ == TEAM_RED ? cache.game_state->ourScore : cache.game_state->opponentScore;
  int blue = cache.robot_state->team_ == TEAM_BLUE ? cache.game_state->ourScore : cache.game_state->opponentScore;
  redScore->setText(QString::number(red));
  redScore->setText(QString::number(blue));
  emit frameLoaded(cache.frame_info->frame_id);
}

void WorldWindow::loadConfig(const ToolConfig& config) {
  wconfig_ = config.worldConfig;
  updateDisplay();
  setMode((WorldMode)wconfig_.mode);
}

void WorldWindow::saveConfig(ToolConfig& config) {
  config.worldConfig = wconfig_;
}

void WorldWindow::controlsChanged() {
  if(loading_) return;
  wconfig_.simPlayers = playersBox->value();
  wconfig_.playSpeed = speedBox->value();
  wconfig_.autoplay = autoplayBox->isChecked();
  wconfig_.teamNumber = teamNumberBox->value();
  wconfig_.teamColor = teamColorBox->currentIndex();
  wconfig_.skip = skipBox->value();
  wconfig_.seed = seedBox->value();
  auto mode = modeBox->currentIndex();
  setMode((WorldMode)mode);
  ConfigWindow::saveConfig();
}

void WorldWindow::setDisplay(bool value) {
  for(int i = 0; i < GLDrawer::NUM_DisplayOptions; i++) 
    simconfig_.options[(GLDrawer::DisplayOption)i] = value;
}

void WorldWindow::stopSimulation() {
  if(inSimMode()) {
    world->setSimulation(nullptr);
    simControl->setSimulation(nullptr);
    if(simulation_) {
      simulation_.reset();
      play_ = false;
    }
  } else if(inLiveMode()) {
    if(livecore_)
      stopLiveMode();
  }
}

void WorldWindow::startSimulation() {
  if(inSimMode()) {
    switch(wconfig_.mode) {
      case BehaviorSim: simulation_ = std::make_unique<BehaviorSimulation>(false); break;
      case BehaviorSimLoc: simulation_ = std::make_unique<BehaviorSimulation>(true); break;
      case LocalizationSim: {
          if(wconfig_.locSimPathfile == "")
            simulation_ = std::make_unique<LocalizationSimulation>(wconfig_.seed);
          else
            simulation_ = std::make_unique<LocalizationSimulation>(wconfig_.locSimPathfile); 
        }
        break;
      case IsolatedBehaviorSim: simulation_ = std::make_unique<IsolatedBehaviorSimulation>(false); break;
      case IsolatedBehaviorSimLoc: simulation_ = std::make_unique<IsolatedBehaviorSimulation>(true); break;
      case GoalieSim: simulation_ = std::make_unique<GoalieSimulation>(); break;
    }
    player_ = simulation_->defaultPlayer();
    if(wconfig_.autoplay) play_ = true;
    world->setSimulation(simulation_.get());
    simControl->setSimulation(simulation_.get());
  } else if(inLiveMode()) {
    startLiveMode();
  }
  if(inSimMode()) {
    skip();
  } else {
    world->updateDisplay(simconfig_.options);
  }
}

void WorldWindow::displayOptionsChanged(bool) {
  if(updating_) return;
  for(int i = 0; i < GLDrawer::NUM_DisplayOptions; i++) {
    auto option = (GLDrawer::DisplayOption)i;
    auto box = displayBoxes_[option];
    simconfig_.options[option] = box->isChecked();
  }
  simconfig_.saveToFile(CONFIG_PATH(wconfig_.mode));
  world->updateDisplay(simconfig_.options);
}

void WorldWindow::setMode(WorldMode mode, bool restart) {
  static auto lastmode = NoDraw;
  if(lastmode == mode && !restart) return;
  stopSimulation();
  wconfig_.mode = lastmode = mode;
  if(this->isVisible())
    startSimulation();
  if(!restart) {
    simconfig_.loadFromFile(CONFIG_PATH(wconfig_.mode));
  }
  updateDisplay();
}

bool WorldWindow::inSimMode() {
  return 
    wconfig_.mode == BehaviorSim ||
    wconfig_.mode == IsolatedBehaviorSim ||
    wconfig_.mode == IsolatedBehaviorSimLoc ||
    wconfig_.mode == BehaviorSimLoc ||
    wconfig_.mode == LocalizationSim ||
    wconfig_.mode == GoalieSim;
}

void WorldWindow::handleStreaming(bool streaming) {
  if(streaming && !inSimMode() && !inLiveMode())
    restart();
}

void WorldWindow::skip() {
  if(inSimMode()) {
    auto cache = simulation_->getGtMemoryCache();
    while(cache.frame_info->frame_id < wconfig_.skip) {
      simulation_->simulationStep();
    }
  }
}

void WorldWindow::restart() {
  stopSimulation();
  setMode((WorldMode)wconfig_.mode, true);
  if(wconfig_.autoplay)
    play();
}

void WorldWindow::forward() {
  if(inSimMode()) {
    simulation_->simulationStep();
    forward_ = true;
    updateSimulationView();
  } else {
    emit nextSnapshot();
    world->updateDisplay(simconfig_.options);
  }
}

void WorldWindow::back() {
  if(!inSimMode()) emit prevSnapshot();
}

void WorldWindow::updateSimulationState() {
  state_timer_->setInterval(1/wconfig_.playSpeed * 1'000);
  if(!play_) return;
  if(!inSimMode()) return;
  forward();
}

void WorldWindow::updateSimulationView() {
  view_timer_->setInterval(1.0f/3.0f * 1'000); // 3 FPS
  if(!play_ && !forward_) return;
  if(!inSimMode()) return;
  auto cache = simulation_->getGtMemoryCache(player_);
  updateMemory(cache);
  if(UTMainWnd::inst()->logWnd_->isVisible()) {
    UTMainWnd::inst()->logWnd_->setText(simulation_->getTextDebug(player_));
    UTMainWnd::inst()->logWnd_->updateFrame(cache.memory);
  }
  UTMainWnd::inst()->walkWnd_->update(cache.memory);
  UTMainWnd::inst()->stateWnd_->update(cache.memory);
  UTMainWnd::inst()->sensorWnd_->update(cache.memory);
  UTMainWnd::inst()->jointsWnd_->update(cache.memory);
  simText->setText(QString::fromStdString(simulation_->getSimInfo()));
  world->updateDisplay(simconfig_.options);
  forward_ = false;
}

void WorldWindow::startLiveMode() {
  if(!livecore_) {
    livecore_ = std::make_unique<VisionCore>(CORE_SIM,false,wconfig_.teamNumber, WO_TEAM_LISTENER);
    livecore_->communications_->initUDP();
    livecore_->textlog_->onlineMode() = true;
    livecache_.fill(livecore_->memory_);
    livecache_.world_object->init();
    if(livecache_.robot_state == nullptr || livecache_.game_state == nullptr) return;
    updateMemory(livecache_);
    livetimer_ = new QTimer(this);
    connect(livetimer_, SIGNAL(timeout()), this, SLOT(updateLiveMode()));
  }
  livecache_.robot_state->team_ = wconfig_.teamColor;
  livecache_.game_state->gameContTeamNum = wconfig_.teamNumber;
  livetimer_->start(1000/30);
  UTMainWnd::inst()->logWnd_->updateFrame(livecache_.memory);
}

void WorldWindow::updateLiveMode() {
  if(!this->isVisible()) return;
  if(UTMainWnd::inst()->logWnd_->isVisible()) {
    auto entries = livecore_->textlog_->textEntries();
    if(entries.size() > 0) {
      UTMainWnd::inst()->logWnd_->setText(entries);
      UTMainWnd::inst()->logWnd_->updateFrame(livecache_.memory, true);
      livecore_->textlog_->textEntries().clear();
    }
  }
  livecache_.frame_info->frame_id++;
  livecore_->communications_->processFrame();
  world->updateDisplay(simconfig_.options);
  updateMemory(livecache_);
}

void WorldWindow::stopLiveMode() {
  if(!livecore_) return;
  livecore_.reset();
  livetimer_->stop();
}

bool WorldWindow::inLiveMode() {
  return wconfig_.mode == (int)Live;
}

void WorldWindow::fieldClicked(Point2D pos, Qt::MouseButton button) {
}

void WorldWindow::fieldHovered(Point2D pos) {
}
  
void WorldWindow::showEvent(QShowEvent* event) {
  if(simulation_ == nullptr && livecore_ == nullptr)
    startSimulation();
}
