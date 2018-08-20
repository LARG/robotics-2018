#ifndef SIM_TRUTH_DATA_H_
#define SIM_TRUTH_DATA_H_

#include <memory/MemoryBlock.h>
#include <math/Pose2D.h>

struct SimTruthDataBlock : public MemoryBlock {
  NO_SCHEMA(SimTruthDataBlock);
public:
  SimTruthDataBlock()
  {
    header.version = 0;
    header.size = sizeof(SimTruthDataBlock);

    has_truth_=false;
  }

  bool has_truth_;
  
  Pose2D robot_pos_;
  Pose2D ball_pos_;
};

#endif 
