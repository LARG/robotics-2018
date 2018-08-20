#ifndef OPP_STRUCT
#define OPP_STRUCT

#include <memory/OpponentBlock.h>
#include <common/Opponent.h>
#include <common/Serialization.h>
#include <schema/gen/OppStruct_generated.h>

DECLARE_INTERNAL_SCHEMA(struct OppStruct {
  SCHEMA_METHODS(OppStruct);
  SCHEMA_FIELD(std::array<Opponent,MAX_OPP_MODELS_IN_MEM> opponents);
});
#endif
