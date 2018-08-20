#include <vision/structures/GoalPostCandidate.h>

//bool sortBlobPixelRatioPredicate(Blob b1, Blob b2){
//  return (b1.correctPixelRatio > b2.correctPixelRatio);
//}

bool sortPostEdgeStrengthPredicate(GoalPostCandidate b1, GoalPostCandidate b2){
  return ((b1.edgeStrength / b1.edgeSize) > (b2.edgeStrength / b2.edgeSize));
}

bool sortPostXiPredicate(GoalPostCandidate b1, GoalPostCandidate b2){
  return (b1.xi < b2.xi);
}

bool overlapping(GoalPostCandidate b1, GoalPostCandidate b2){
  if (b1.xi > b2.xf || b2.xi > b1.xf){
    return false;
  }
  if (b1.yi > b2.yf || b2.yi > b1.yf){
    return false;
  }
  return true;
}






