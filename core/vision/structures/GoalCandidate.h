#ifndef GOAL_CANDIDATE_H
#define GOAL_CANDIDATE_H

#include <vision/structures/GoalPostCandidate.h>
#include <stdlib.h>

struct GoalCandidate {

  GoalPostCandidate leftPost;
  GoalPostCandidate rightPost;
  float width;        // Distance between the posts


};


bool sortGoalPredicate(const GoalCandidate &c1, const GoalCandidate &c2);

#endif
