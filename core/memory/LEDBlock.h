#ifndef LEDBLOCK_
#define LEDBLOCK_

#include <iostream>

#include <memory/MemoryBlock.h>
#include <common/RobotInfo.h>
#include <schema/gen/LEDBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct LEDBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(LEDBlock);
    LEDBlock()  {
      header.version = 1;
      header.size = sizeof(LEDBlock);
      
      for (int i=0; i<NUM_LEDS; i++) {
        values_[i]=0.0;
      }
      send_leds_ = true;
    }
    SCHEMA_FIELD(bool send_leds_);
    SCHEMA_FIELD(std::array<float,NUM_LEDS> values_);
});

#endif 
