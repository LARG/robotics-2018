#include "CoachSimulation.h"
#include <common/FieldConfiguration.h>
#include <tool/UTMainWnd.h>

typedef IsolatedBehaviorSimulation IBSim;
typedef ObjectConfiguration OP;

#define _X 0
#define _Y (-HALF_FIELD_Y - 750)
#define _ORIENT (M_PI/2)
#define _HEIGHT 700

CoachSimulation::CoachSimulation() : IBSim(false, WO_TEAM_COACH) {
  FieldConfiguration config;
  if(!config.loadFromFile(UTMainWnd::dataDirectory() + "/csim_config.yaml")) {
    config = {
      {player_, OP(_X, _Y, _ORIENT, _HEIGHT)},
      {WO_BALL, OP(0, 0)}
    };
  }
  config.saveToFile(UTMainWnd::dataDirectory() + "/csim_config.yaml");
  bcache_.robot_state->manual_pose_ = Pose2D(_ORIENT, _X, _Y);
  bcache_.robot_state->manual_height_ = _HEIGHT;
  bcache_.robot_state->role_ = WO_TEAM_COACH;
  gtcache_.game_state->setState(PLAYING);
  bcache_.game_state->setState(PLAYING);
  config.place(bcache_.world_object);
  config.place(gtcache_.world_object);
  physics_.setObjects(gtcache_.world_object);
}
