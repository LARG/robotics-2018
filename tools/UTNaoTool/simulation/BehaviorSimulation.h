#ifndef BEHAVIOR_SIM_H
#define BEHAVIOR_SIM_H

#include <vector>
#include <string>

#include <math/Pose2D.h>
#include <math/Geometry.h>
#include <common/WorldObject.h>
#include <common/States.h>
#include "SimulatedPlayer.h"
#include "Simulation.h"
#include "PhysicsSimulator.h"

class WorldObjectBlock;
class GameStateBlock;
class FrameInfoBlock;

class WorldObject;
class VisionCore;
class MemoryFrame;

class BehaviorSimulation : public Simulation {

  public:
    BehaviorSimulation(int nplayers, bool penaltyKick, bool locMode);
    ~BehaviorSimulation();

    void simulationStep();
    
    int defaultPlayer() { return activePlayers_[0]; }
    MemoryCache getGtMemoryCache(int player);
    MemoryCache getBeliefMemoryCache(int player);
    std::vector<std::string> getTextDebug(int id);
    void setPenalty(int index);
    void setStrategy();
    void setFallen(int index);
    void flipRobot(int index);
    void changeSimulationState(State state);
    void changeSimulationKickoff();
    void moveRobot(int index, AngRad rotation, Point2D movement);
    void moveBall(Point2D translation);
    void restart();
    int checkLocalizationErrors();
    void runParamTests();
    void kickBall();

    bool complete();

    std::string getSimInfo();
    std::string simInfo;
    
    bool lmode() { return lmode_; }
    int numHalves;
    int simBlueScore;
    int simRedScore;
    bool forceManualPositions;
    bool forceDesiredPositions;
    WorldObjectBlock* worldObjects;
    GameStateBlock* gameState;
    FrameInfoBlock* frameInfo;
    SimulatedPlayer* sims[WO_ROBOTS_LAST + 1];
    bool simActive[WO_ROBOTS_LAST + 1];
  private:

    void restartSimulationInterpreter();
    void doPenaltyKickReset();

    void stepBall(Point2D &ballLoc, Point2D &ballVel);
    void stepTimer(Point2D &ballLoc, Point2D &ballVel);
    void stepCheckGoal(Point2D &ballLoc, Point2D &ballVel);
    void stepCheckBounds(Point2D &ballLoc, Point2D &ballVel);
    void stepCheckBallCollisions(Point2D &ballLoc, Point2D &ballVel);
    void stepPlayerFallen(int i);
    void stepPlayerBounds(int i);
    void stepPlayerKick(int i);
    void stepPlayerBumpBall(int i, Point2D &ballLoc, Point2D &ballVel, WorldObject *robot);
    void stepPlayerPenaltyBox(int i, WorldObject *robot);
    void stepPlayerCollisions(int i, WorldObject *robot);
    void stepPlayerComm(int i);

    void getTeamSignAndSelf(int i, int &teamsign, int &self);
    void setObjectFromPose(int i, int teamsign, const Pose2D *pose);


    // for comparing behavior params
    //void compareParams();
    //void differParams(int param);
    //void differBHumanParams(int param);
    void runParamTest();

    void runKickTests();

    WorldObject* getPlayerObject(int id);

    bool lmode_;
    float fallenTime[WO_ROBOTS_LAST + 1];
    int lastKick;
    float lastKickX;

    // sim variables
    int currentSim;
    float simTimer;
    float halfTimer;
    bool simPenaltyKick;
    bool ballClearedFromCircle;
    bool simOn;
    int nplayers;
    float timeInc;
    bool PRINT;

    void setSimScore(bool blue);

    MemoryFrame* memory_;
    PhysicsSimulator physics_;
};


#endif
