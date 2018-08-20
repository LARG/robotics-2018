#ifndef DELAYEDLOCALIZATIONBLOCK_H_47YLPZHE
#define DELAYEDLOCALIZATIONBLOCK_H_47YLPZHE

#include <memory/MemoryBlock.h>
#include <math/Pose2D.h>

struct DelayedLocalizationBlock: public MemoryBlock {
  NO_SCHEMA(DelayedLocalizationBlock);
  DelayedLocalizationBlock():
    hasUpdate(false),
    frameStarted(0),
    conf(-1),
    odometryDisplacement(0,0,0)
  {
    header.version = 1;
    header.size = sizeof(DelayedLocalizationBlock);
  };

  void reset(unsigned int frameId) {
    hasUpdate = false;
    conf = -1;
    frameStarted = frameId;
    odometryDisplacement = Pose2D(0,0,0);
  }

  void set(const Pose2D &nestimatedPose, float nconf) {
    hasUpdate = true;
    conf = nconf;
    estimatedPose = nestimatedPose;
    odometryDisplacement = odometryAccumulator;
    odometryAccumulator = Pose2D(0,0,0);
  }

  bool hasUpdate;
  unsigned int frameStarted; // not really needed now, but might be interesting
  float conf; // confidence in prediction
  Pose2D odometryDisplacement; // odometry displacement since the processing started, final result
  Pose2D estimatedPose; // estimated pose by the delayed localization update
  Pose2D odometryAccumulator; // accumulate the odometry, intermediate result

};

#endif /* end of include guard: DELAYEDLOCALIZATIONBLOCK_H_47YLPZHE */

