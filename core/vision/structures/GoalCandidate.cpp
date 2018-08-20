
#include <vision/structures/GoalCandidate.h>

// TODO: Use width from FIELD.H

bool sortGoalPredicate(const GoalCandidate &c1, const GoalCandidate &c2){
  float c1Error = abs(c1.width - 1620);
  float c2Error = abs(c2.width - 1620);
  return c1Error < c2Error;
}
