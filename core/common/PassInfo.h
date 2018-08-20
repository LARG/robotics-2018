#ifndef PASS_INFO_H
#define PASS_INFO_H

#include <common/Serialization.h>
#include <schema/gen/PassInfo_generated.h>
#include <math/Geometry.h>

DECLARE_INTERNAL_SCHEMA(struct PassInfo {
  SCHEMA_METHODS(PassInfo);
  SCHEMA_FIELD(bool executingPass);
  SCHEMA_FIELD(int targetPlayer);
  SCHEMA_FIELD(Point2D target);
  SCHEMA_FIELD(float timeUntilPass);
  SCHEMA_FIELD(int kickChoice);

  PassInfo():
    executingPass(false),
    targetPlayer(-1)
  {
  }
});
#endif
