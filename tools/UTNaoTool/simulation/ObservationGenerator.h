#ifndef OBSERVATION_GENERATOR_H
#define OBSERVATION_GENERATOR_H

#include <memory/MemoryCache.h>
#include <math/Geometry.h>
#include <common/Random.h>
#include <common/RobotInfo.h>
#include <common/ImageParams.h>


class ObservationGenerator {
  public:
    ObservationGenerator();
    ~ObservationGenerator();
    ObservationGenerator(const ObservationGenerator& og);
    void setObjectBlocks(WorldObjectBlock* gtObjects, WorldObjectBlock* obsObjects);
    void setObjectBlocks(WorldObjectBlock* gtObjects, std::vector<WorldObjectBlock*> obsObjects);
    void setInfoBlocks(FrameInfoBlock* frameInfo, JointBlock* joints);
    void setModelBlocks(OpponentBlock* opponentMem);
    void setPlayer(int player, int team);
    void setImageParams(const ImageParams& iparams);
    void generateAllObservations();
    void generateGroundTruthObservations();
  private:
    void generateBallObservations();
    void generateLineObservations();
    void generateOpponentObservations();
    void generateCenterCircleObservations();
    void generateGoalObservations();
    void generatePenaltyCrossObservations();
    void generateBeaconObservations();
    void fillObservationObjects();
    ImageParams iparams_;
    Random rand_;
    WorldObjectBlock *gt_object_, *obs_object_;
    OpponentBlock* opponent_mem_;
    FrameInfoBlock* frame_info_;
    JointBlock* joint_;
    int player_, team_;
    std::vector<WorldObjectBlock*> obs_objects_;
};

#endif
