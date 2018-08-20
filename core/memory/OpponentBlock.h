#ifndef OPPONENTBLOCK_
#define OPPONENTBLOCK_

#include <memory/MemoryBlock.h>
#include <math/Geometry.h>
#include <common/CommStruct.h>

#define MAX_OPP_MODELS_IN_MEM 5 // TODO: change team packets oppData to use MAX_OPP_MODELS_IN_MEM, increase this back up to 8

struct OpponentModel {
  int modelNumber;
  float alpha;
  float
    X00,
    X10,
    X20,
    X30,
    P00,
    P10,
    P01,
    P11,
    P22,
    P33;
  int frameLastObserved;
  float SRXX, SRXY, SRYY;
  Point2D loc, sd, vel;
};

struct OpponentBlock : public MemoryBlock {
  NO_SCHEMA(OpponentBlock);
public:
  OpponentBlock()  {
    header.version = 3;
    header.size = sizeof(OpponentBlock);
  }

  OpponentModel 
    locModels[MAX_OPP_MODELS_IN_MEM], // Models used by localization (in cm)
    genModels[MAX_OPP_MODELS_IN_MEM]; // Models used everywhere else (in mm)

  void syncModels() {
    for(int i = 0; i < MAX_OPP_MODELS_IN_MEM; i++) {
      OpponentModel &gen = genModels[i], &loc = locModels[i];
      for(float *g = &gen.X00, *l = &loc.X00; g <= &gen.P33; g++, l++)
        *g = *l * 10;

      gen.loc = Point2D(gen.X00, gen.X10);
      gen.sd = Point2D(gen.P00, gen.P11);
      gen.vel = Point2D(gen.X20, gen.X30);
    }
  }

  OpponentModel getModel(int index) {
    return genModels[index];
  }
};
#endif 
