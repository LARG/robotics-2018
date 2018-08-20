#include <tool/WorldWindow.h>
#include <common/annotations/AnnotationGroup.h>
#include <tool/simulation/BehaviorSimulation.h>
#include <tool/simulation/LocalizationSimulation.h>
#include <tool/simulation/IsolatedBehaviorSimulation.h>
#include <tool/simulation/CoachSimulation.h>
#include <tool/simulation/GoalieSimulation.h>
#include <communications/CommunicationModule.cpp>

#define CONFIG_PATH(mode) (UTMainWnd::dataDirectory() + "/simulators/" + getName((WorldMode)(mode)) + ".yaml")
using namespace std;

WorldWindow::WorldWindow(QMainWindow* pa) : ConfigWindow(pa) {
  simulation_ = NULL;
  livecore_ = NULL;
  annotations_ = NULL;
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
  play_ = false; wconfig_.playSpeed = 50;
  playTimer_ = new QTimer(this);
  connect(playTimer_, SIGNAL(timeout()), this, SLOT(updateSimulation()));
  playTimer_->start();
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
    if(simulation_) {
      delete simulation_;
      simulation_ = NULL;
      play_ = false;
    }
    world->setSimulation(simulation_);
    simControl->setSimulation(simulation_);
  } else if(inLiveMode()) {
    if(livecore_)
      stopLiveMode();
  }
}

void WorldWindow::startSimulation() {
  if(inSimMode()) {
    switch(wconfig_.mode) {
      case BehaviorSim: simulation_ = new BehaviorSimulation(wconfig_.simPlayers, false, false); break;
      case BehaviorSimLoc: simulation_ = new BehaviorSimulation(wconfig_.simPlayers, false, true); break;
      case LocalizationSim: {
          if(wconfig_.locSimPathfile == "")
            simulation_ = new LocalizationSimulation(wconfig_.seed);
          else
            simulation_ = new LocalizationSimulation(wconfig_.locSimPathfile); 
        }
        break;
      case IsolatedBehaviorSim: simulation_ = new IsolatedBehaviorSimulation(false); break;
      case IsolatedBehaviorSimLoc: simulation_ = new IsolatedBehaviorSimulation(true); break;
      case CoachSim: simulation_ = new CoachSimulation(); break;
      case GoalieSim: simulation_ = new GoalieSimulation(); break;
    }
    player_ = simulation_->defaultPlayer();
    if(wconfig_.autoplay) play_ = true;
    world->setSimulation(simulation_);
    simControl->setSimulation(simulation_);
  } else if(inLiveMode()) {
    startLiveMode();
  }
  if(inSimMode()) {
    skip();
    updateSimulationView();
    simText->setText(QString::fromStdString(simulation_->getSimInfo()));
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
    wconfig_.mode == CoachSim ||
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
    updateSimulationView();
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
    updateSimulationView();
    simText->setText(QString::fromStdString(simulation_->getSimInfo()));
  } else {
    emit nextSnapshot();
    world->updateDisplay(simconfig_.options);
  }
}

void WorldWindow::back() {
  if(!inSimMode()) emit prevSnapshot();
}

void WorldWindow::updateSimulation() {
  playTimer_->setInterval(1/wconfig_.playSpeed * 1000);
  if(!play_) return;
  if(!inSimMode()) return;
  forward();
}

void WorldWindow::updateSimulationView() {
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
  world->updateDisplay(simconfig_.options);
}

void WorldWindow::startLiveMode() {
  if(!livecore_) {
    livecore_ = new VisionCore(CORE_SIM,false,wconfig_.teamNumber, WO_TEAM_LISTENER);
    livecore_->communications_->initUDP();
    livecache_.fill(livecore_->memory_);
    livecache_.world_object->init();
    if(livecache_.robot_state == NULL || livecache_.game_state == NULL) return;
    updateMemory(livecache_);
    livetimer_ = new QTimer(this);
    connect(livetimer_, SIGNAL(timeout()), this, SLOT(updateLiveMode()));
  }
  livecache_.robot_state->team_ = wconfig_.teamColor;
  livecache_.game_state->gameContTeamNum = wconfig_.teamNumber;
  livetimer_->start(1000/30);
}

void WorldWindow::updateLiveMode() {
  livecache_.frame_info->frame_id++;
  world->updateDisplay(simconfig_.options);
  updateMemory(livecache_);
}

void WorldWindow::stopLiveMode() {
  if(!livecore_) return;
  livetimer_->stop();
}

bool WorldWindow::inLiveMode() {
  return wconfig_.mode == (int)Live;
}

void WorldWindow::fieldClicked(Point2D pos, Qt::MouseButton button) {
}

void WorldWindow::fieldHovered(Point2D pos) {
}
