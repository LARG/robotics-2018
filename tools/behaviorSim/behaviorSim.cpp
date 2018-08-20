#include <stdio.h>
#include <string.h>
#include "../UTNaoTool/BehaviorSimulation.h"
#include <lua/LuaModule.h>
#include <VisionCore.h>

int main(int argc, char **argv) {

  if (argc != 2){
    cout << "Please provide a red team config file" << endl;
    exit(0);
  }
  
  std::string cmd = std::string("roleSwitch.setupRoles('") + argv[1] + "')";
  BehaviorSimulation behaviorSim(WO_OPPONENT_LAST,false,false);
  for (int i = 1; i <= WO_OPPONENT_LAST; i++) {
    if (behaviorSim.sims[i] != NULL)
      //behaviorSim.sims[i]->core->interpreter_->call(cmd);
  }
  if ((((float)rand())/((float)RAND_MAX)) < 0.5) {
    behaviorSim.changeSimulationKickoff();
  }
  while (behaviorSim.numHalves < 1) {
    behaviorSim.simulationStep();
    //if (((int)behaviorSim.halfTimer % 50) == 0)
      //std::cout << behaviorSim.halfTimer << std::endl;
  }
  std::cout << "Score: " << behaviorSim.simBlueScore << " " << behaviorSim.simRedScore << std::endl;
  return 0;

}
