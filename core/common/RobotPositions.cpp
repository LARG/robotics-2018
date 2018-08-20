#include "RobotPositions.h"

Pose2D* RobotPositions::startingSidelinePoses = new Pose2D[NUM_TEAM_POSES];
Pose2D* RobotPositions::ourKickoffPosesDesired = new Pose2D[NUM_TEAM_POSES];
Pose2D* RobotPositions::ourKickoffPosesManual = new Pose2D[NUM_TEAM_POSES];
Pose2D* RobotPositions::penaltyPoses = new Pose2D[NUM_PENALTY_POSES];
bool* RobotPositions::ourKickoffPosesManualReversible = new bool[NUM_TEAM_POSES];
Pose2D* RobotPositions::theirKickoffPosesDesired = new Pose2D[NUM_TEAM_POSES];
Pose2D* RobotPositions::theirKickoffPosesManual = new Pose2D[NUM_TEAM_POSES];
bool* RobotPositions::theirKickoffPosesManualReversible = new bool[NUM_TEAM_POSES];
