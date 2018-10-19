#include "LocalizationSimulation.h"
#include <localization/LocalizationModule.h>
#include <common/File.h>
#include <sstream>
#include <common/Util.h>

#define SECONDS_PER_FRAME (1.0/30.0)
#define RADS_PER_FRAME (DEG_T_RAD * 1.0)
#define DISTANCE_PER_FRAME 5.0

const string PATH_DIR = string(getenv("NAO_HOME")) + "/data/paths";

#define getObject(obj, idx, cache) \
  auto& obj = cache.world_object->objects_[idx];

#define player_ 1
#define team_ 0

LocalizationSimulation::LocalizationSimulation(string pathfile) : iparams_(Camera::TOP), cg_(team_, player_) {
  vector<LocSimAgent::Type> types = { LocSimAgent::Type::Default };
  init(types);
  SimulationPath path;
  path.loadFromFile(PATH_DIR + "/" + pathfile);
  printf("Loaded simulation path with %i points from file: %s\n", path.size(), pathfile.c_str());
  setPath(path);
}

LocalizationSimulation::LocalizationSimulation(int seed) : iparams_(Camera::TOP), cg_(team_, player_), seed_(seed)  {
  vector<LocSimAgent::Type> types = { LocSimAgent::Type::Default };
  init(types);
}

LocalizationSimulation::LocalizationSimulation(LocSimAgent::Type type) : iparams_(Camera::TOP), cg_(team_, player_) {
  vector<LocSimAgent::Type> types = { type };
  init(types);
}

LocalizationSimulation::~LocalizationSimulation() {
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    delete agent.core;
  }
}

void LocalizationSimulation::init(vector<LocSimAgent::Type> types) {
  path_ = SimulationPath::generate(10, seed_);
  gtcache_ = MemoryCache::create(team_, player_);
  og_.setInfoBlocks(gtcache_.frame_info, gtcache_.joint);
  og_.setPlayer(player_, team_);
  for(auto type : types) {
    agents_[type] = LocSimAgent(type);
  }
  for(auto& kvp : agents_) {
    activePlayers_.push_back(static_cast<int>(kvp.first));
    startCore(kvp.second);
  }
  ballmove_ = 0;
  outputBadPaths_ = false;
}

MemoryCache LocalizationSimulation::getGtMemoryCache(int) const {
  return gtcache_;
}

MemoryCache LocalizationSimulation::getBeliefMemoryCache(int player) const {
  auto t = static_cast<LocSimAgent::Type>(player);
  auto it = agents_.find(t);
  if(it == agents_.end()) {
    throw std::runtime_error(util::format("Invalid player requested from localization simulation: %s\n", it->first));
  }
  return it->second.cache;
}

void LocalizationSimulation::setPath(const SimulationPath& path) {
  origPath_ = path_ = path;
}

void LocalizationSimulation::flip() {
  getObject(gtSelf, player_, gtcache_);
  gtSelf.loc = -gtSelf.loc;
  gtSelf.orientation += M_PI;
  getObject(gtBall, WO_BALL, gtcache_);
  gtBall.loc = -gtBall.loc;
  path_.flip();
}

void LocalizationSimulation::moveBall() {
  int frame = gtcache_.frame_info->frame_id;
  if(frame - ballmove_ < 450) return;
  auto& ball = gtcache_.world_object->objects_[WO_BALL].loc;
  ball.x += rand_.sampleU(-1000,1000);
  ball.y += rand_.sampleU(-1000,1000);
  ball.x = min(max(ball.x, -FIELD_X/2), FIELD_X/2);
  ball.y = min(max(ball.x, -FIELD_Y/2), FIELD_Y/2);
  ballmove_ = frame;
}

void LocalizationSimulation::moveBall(Point2D position) {
  teleportBall(position);
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    agent.core->localization_->moveBall(position);
  }
}

void LocalizationSimulation::teleportBall(Point2D position) {
  getObject(ball, WO_BALL, gtcache_);
  ball.loc = position;
}

void LocalizationSimulation::movePlayer(Point2D position, float orientation, int) {
  teleportPlayer(position, orientation);
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    agent.core->localization_->movePlayer(position, orientation);
  }
}

void LocalizationSimulation::teleportPlayer(Point2D position, float orientation, int) {
  getObject(self, player_, gtcache_);
  self.loc = position;
  self.orientation = orientation;
}

void LocalizationSimulation::simulationStep() {
  gtcache_.frame_info->frame_id++;
  for(auto& kvp : agents_)  {
    auto& agent = kvp.second;
    agent.cache.frame_info->frame_id = gtcache_.frame_info->frame_id;
    agent.core->textlog_->textEntries().clear();
  }

  stepPose();
  moveBall();
  generateObservations();
  generateCommunications();
  processLocalizationFrame();
  stepError();
}

void LocalizationSimulation::placeObjects(LocSimAgent& agent) {
  auto& cache = agent.cache;
  getObject(gtBall, WO_BALL, gtcache_);
  getObject(obsBall, WO_BALL, cache);
  gtBall.loc = obsBall.loc = Point2D(2000,750);

  getObject(gtSelf, player_, gtcache_);
  getObject(obsSelf, player_, cache);
  gtSelf.loc = obsSelf.loc = path_.lastPoint();
  auto& pose = RobotPositions::startingSidelinePoses[player_];
  pose.translation.x = gtSelf.loc.x;
  pose.translation.y = gtSelf.loc.y;
  pose.rotation = 0;
}

void LocalizationSimulation::startCore(LocSimAgent& agent) {
  agent.steps = 0;
  auto& core = agent.core;
  auto& cache = agent.cache;
  core = new VisionCore(CORE_TOOLSIM,false,team_,player_, agent.method);
  core->textlog_->onlineMode() = true;
  cache.fill(core->memory_);
  cache.world_object->init();
  cache.game_state->setState(PLAYING);
  core->interpreter_->start();
  placeObjects(agent);
  core->localization_->reInit();
}

void LocalizationSimulation::stepError() {
  getObject(gtself, player_, gtcache_);
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    getObject(bself, player_, agent.cache);
    float dist = bself.loc.getDistanceTo(gtself.loc);
    float rot = fabs(bself.orientation - gtself.orientation);
    while(rot > M_PI) rot = fabs(rot - 2 * M_PI);
    agent.distError += dist*dist;
    agent.rotError += rot * rot * RAD_T_DEG * RAD_T_DEG;
    agent.steps += 1;
  }
  if(complete() && outputBadPaths_ && agents_.find(LocSimAgent::Type::Default) != agents_.end()) {
    auto& agent = agents_[LocSimAgent::Type::Default];
    if(agent.distRMSE() > maxDistError_ || agent.rotRMSE() > maxRotError_ || agent.distRMSE() != agent.distRMSE()) {
      string timestamp = generateTimestamp();
      stringstream ss;
      char buf[100];
      sprintf(buf, "path_D_%2.f_R_%2.f", agent.distRMSE(), agent.rotRMSE());
      ss << PATH_DIR << "/" << buf << timestamp << ".yaml";
      string filename = ss.str();
      origPath_.saveToFile(filename);
    }
  }
}

void LocalizationSimulation::stepPose() {
  if(path_.empty()) return;
  getObject(self, player_, gtcache_);

  self.orientation = normalizeAngle(self.orientation);

  auto cpoint = self.loc;
  auto target = path_.currentPoint();
  auto diff = (target - cpoint);
  auto dist = diff.getMagnitude();
  Pose2D disp;

  // Turn if we're not facing the target point
  bool moveForward = true;
  if(dist > .0001) {
    float tbearing = cpoint.getBearingTo(target, self.orientation);
    if(abs(tbearing) < RADS_PER_FRAME) {
      disp.rotation = tbearing;
      self.orientation += disp.rotation;
    } else {
      float direction = tbearing > 0 ? 1 : -1;
      disp.rotation = direction * RADS_PER_FRAME;
      self.orientation += disp.rotation;
      moveForward = false;
    }
  }
  
  if(moveForward) {
    auto toffset = target - cpoint;
    if(toffset.getMagnitude() < DISTANCE_PER_FRAME) {
      self.loc = target;
      disp.translation.x = (target - self.loc).getMagnitude();
      path_.pop();
    } else {
      auto direction = diff / diff.getMagnitude();
      self.loc += direction * DISTANCE_PER_FRAME;
      disp.translation.x = DISTANCE_PER_FRAME;
    }
  }
  disp.rotation *= pow(2.0, rand_.sampleU(-2.0,2.0));
  disp.translation *= rand_.sampleU(.5,2.0);
  for(auto kvp : agents_) {
    auto& agent = kvp.second;
    agent.cache.odometry->displacement = disp;
    agent.cache.odometry->standing = false;
  }
}

void LocalizationSimulation::generateObservations() {
  vector<WorldObjectBlock*> objects;
  for(auto kvp : agents_) {
    auto woblock = kvp.second.cache.world_object;
    woblock->reset();
    objects.push_back(woblock);
  }
  og_.setObjectBlocks(gtcache_.world_object, objects);
  og_.generateAllObservations();
}

void LocalizationSimulation::generateCommunications() {
  vector<MemoryCache> acaches;
  for(auto& kvp : agents_) {
    acaches.push_back(kvp.second.cache);
  }
  cg_.setCaches(gtcache_, acaches);
  cg_.generateAllCommunications();
}

void LocalizationSimulation::processLocalizationFrame() {
  for(auto& kvp : agents_) {
    auto& core = kvp.second.core;
    core->localization_->processFrame();
  }
}

bool LocalizationSimulation::complete() {
  return path_.empty();
}

AgentError LocalizationSimulation::getError(LocSimAgent::Type type) {
  AgentError e;
  auto& agent = agents_[type];
  e.dist = sqrtf(agent.distError/agent.steps);
  e.rot = sqrtf(agent.rotError/agent.steps);
  e.steps = agent.steps;
  return e;
}

void LocalizationSimulation::printError() {
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    fprintf(stderr, "%s RMSE dist error: %2.2f, rot error: %2.2f, steps: %i\n",
      agent.name().c_str(), sqrtf(agent.distError / agent.steps), sqrtf(agent.rotError / agent.steps), agent.steps);
  }
}

string LocalizationSimulation::getSimInfo() {
  const int maxsize = 1000;
  char buffer[maxsize];
  string info;
  for(auto& kvp : agents_) {
    auto& agent = kvp.second;
    snprintf(buffer, maxsize, "%s RMSE dist error: %2.2f, rot error: %2.2f, steps: %i\n",
      agent.name().c_str(), sqrtf(agent.distError / agent.steps), sqrtf(agent.rotError / agent.steps), agent.steps);
    info += buffer;
  }
  return info;
}

int LocalizationSimulation::defaultPlayer() {
  return static_cast<int>(LocSimAgent::Type::Default);
}

vector<string> LocalizationSimulation::getTextDebug(int player) {
  if(agents_.find(LocSimAgent::Type::Default) == agents_.end()) return vector<string>();
  auto agent = agents_[LocSimAgent::Type::Default];
  if(!agent.core) return vector<string>();
  return agent.core->textlog_->textEntries();
}

void LocalizationSimulation::outputBadPaths(float maxDistError, float maxRotError) {
  maxDistError_ = maxDistError;
  maxRotError_ = maxRotError;
  outputBadPaths_ = true;
  origPath_ = path_;
}
