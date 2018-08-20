#ifndef OPPONENT_H
#define OPPONENT_H

#include <common/Serialization.h>
#include <schema/gen/Opponent_generated.h>

DECLARE_INTERNAL_SCHEMA(struct Opponent { 
  SCHEMA_METHODS(Opponent);
  SCHEMA_FIELD(float sdx = 10000);
  SCHEMA_FIELD(float sdy = 10000);
  SCHEMA_FIELD(float sdxy = 10000);
  SCHEMA_FIELD(int16_t x = 0);
  SCHEMA_FIELD(int16_t y = 0);
  SCHEMA_FIELD(uint16_t framesMissed = 1000);
  SCHEMA_FIELD(bool filled = false);
});
#endif
