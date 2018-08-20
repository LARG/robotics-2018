#ifndef ROBOTINFOBLOCK_CBJC25GF
#define ROBOTINFOBLOCK_CBJC25GF

#include <common/RobotDimensions.h>
#include <common/MassCalibration.h>

#include <memory/MemoryBlock.h>

#include <schema/gen/RobotInfoBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct RobotInfoBlock : public MemoryBlock {
  SCHEMA_METHODS(RobotInfoBlock);
public:
  RobotInfoBlock()  {
    header.version = 1;
    header.size = sizeof(RobotInfoBlock);
  }  

  // These never change and don't need to be serialized per frame
  RobotDimensions dimensions_;
  MassCalibration mass_calibration_;

});

#endif /* end of include guard: ROBOTINFOBLOCK_CBJC25GF */

