#include <tool/simulation/GoalieSimulation.h>
#include <common/FieldConfiguration.h>
#include <tool/UTMainWnd.h>

typedef IsolatedBehaviorSimulation IBSim;
typedef ObjectConfiguration OP;

#define _X (-HALF_FIELD_X + 250)
#define BALL_X (_X + 1500)
#define _Y 0
#define _ORIENT 0
#define getObject(obj,idx) \
  auto& obj = gtcache_.world_object->objects_[idx];

GoalieSimulation::GoalieSimulation() : IBSim(true, KEEPER) {
  auto& config = config_.objects.gtconfig;
//  if(not loadGame("goalie_sim")) {
    std::cout << "Default object placement" << std::endl;
    config = {
      {player_, OP(_X, _Y, _ORIENT)},
      {WO_BALL, OP(BALL_X, _Y)}
    };
//  }
  saveGame("goalie_sim");
  bcache_.robot_state->role_ = KEEPER;
  gtcache_.game_state->setState(config_.game_state);
  bcache_.game_state->setState(config_.game_state);
  applyConfig(gtcache_, {bcache_});
  physics_.setObjects(gtcache_.world_object);
  align_ = true;
  sframe_ = 0;
  sim_.initLocalization();
}

void GoalieSimulation::simulationStep() {
  physics_.step();
  int frames = ++gtcache_.frame_info->frame_id - sframe_;
  if(frames > 150) {
    if(align_)
      kickBall();
    else
      resetBall();
    sframe_ = gtcache_.frame_info->frame_id;
    align_ = !align_;
  }
  sim_.processFrame(gtcache_.world_object, gtcache_.game_state);
}

void GoalieSimulation::kickBall() {
  rand_ = Random(time(NULL));
  getObject(ball, WO_BALL);
  getObject(goal, WO_OWN_GOAL);
  float directionToGoal = ball.loc.getAngleTo(goal.loc);
  float direction = normalizeAngle(rand_.sampleU(-30, 30) * DEG_T_RAD + directionToGoal);
  float magnitude = rand_.sampleU(1000,3000);
  ball.absVel = Point2D::getPointFromPolar(magnitude, direction);
}

void GoalieSimulation::resetBall() {
  getObject(ball, WO_BALL);
  ball.loc = Point2D(BALL_X, rand_.sampleU(-1000,1000));
}
