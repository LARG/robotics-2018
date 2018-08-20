#include <vision/structures/RobotCandidate.h>

bool RobotCandidate::sortPredicate(const RobotCandidate& left, const RobotCandidate& right) {
  return left.confidence > right.confidence;
}
