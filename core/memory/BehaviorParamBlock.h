#ifndef BEHAVIORPARAMBLOCK_
#define BEHAVIORPARAMBLOCK_

#include <memory/MemoryBlock.h>
#include <math/Geometry.h>
#include <common/Kicks.h>
#include <common/Roles.h>
#include <common/WorldObject.h>

class Cluster { // class for python access, otherwise namespace
 public:
  
  enum Activity {
    NONE,
    DRIBBLE,
    QUICK,
    SIDEKICK
  };
};

const unsigned int NUM_FIELD_PLAYERS = WO_TEAM_LAST - WO_TEAM_FIELD_FIRST + 1;

struct RolePositionConfig {
  Point2D offsetFromBall;
  float maxY;
  float minX;
  float maxX;
  bool splitY;
  bool oppositeYOfBall;
};

struct FieldAreaRoleConfig {
  float minX;
  int rolesOrderedByImportance[NUM_FIELD_PLAYERS]; // chaser usually ignored, but used when goalie is clearing
  FieldAreaRoleConfig() {
    for (unsigned int i = 0; i < NUM_FIELD_PLAYERS; i++)
      rolesOrderedByImportance[i] = -1;
  }

  void setRole(int i, int role) {
    rolesOrderedByImportance[i] = role;
  }
};


const unsigned int MAX_NUM_FIELD_AREAS = 6;
struct RoleStrategy {
  FieldAreaRoleConfig clearingKeeperArea;
  FieldAreaRoleConfig kickoffArea;
  FieldAreaRoleConfig fieldAreas[MAX_NUM_FIELD_AREAS];
  RolePositionConfig rolePositionConfigs[NUM_Roles];

  RolePositionConfig* getRolePositionConfigPtr(int i) {
    return rolePositionConfigs + i;
  }

  FieldAreaRoleConfig* getFieldAreaRoleConfigPtr(int i) {
    return fieldAreas + i;
  }
};

struct KickStrategy {
  float edgeBuffer;
  float postAngle;
  float forwardOpeningAngle;
  float insidePostBuffer;
  float maxArcAngle;
  float shootOnGoalRadius;
  float ownGoalRadius;
  float orientationErrorFactor;
  float maxOpponentSD; // biggest sd for opponent before we ignore
  float minOpponentDist; // passes must be at least this far from opponent
  float allowOpponentSideDist; // passes that are more than this to the side can ignore the minOpponentDist (negative disallows this option)
  float opponentWidth; // make sure ball clears at least this far from each opp
  float passDistance;
  float maxKickAngle; // max angle we can kick (or want to try)
  // how far offset these positions should be from the ball
  //float supportFwdDist;
  //float supportSideDist;
  //float defendBackDist;
  //float forwardFwdDist;
  //float forwardSideDist;
  float minOppDistForSlowKick;
  float minOppDistForSuperKick;
  float minOppDistForExtraRotateOffset; // const dist factor added to the below
  float minOppDistForExtraRotateFactor; // conversion from goalBearing in radians into distance
  bool usableKicks[NUM_KICKS];
  int defaultKick;
  //bool useSupporterAtMidfield;
  
  float ignoreBehindAngleForSlowKick;

	void setUsableKick(int i, bool usable) { usableKicks[i] = usable;};
  bool getUsableKick(int i){return usableKicks[i];};

  KickStrategy(){}
};

struct ClusterKickStrategy {
  Cluster::Activity behavior;
  // negative values disables using these over the regular KickStrategy params
  float forwardOpeningAngle;
  float allowOpponentSideDist;
  float shootOnGoalRadius;

  ClusterKickStrategy() {}
  static float getValue(bool cluster, float origVal, float clusterVal) {
    if (cluster && (clusterVal >= 0))
      return clusterVal;
    return origVal;
  }
};

struct CornerKickStrategy {
  float cornerAngle;
  float endlineBuffer;
  float distFromEndline;
  float distFromFarPost;
  Point2D targetPt;
  float orientationErrorFactor;
  bool usableKicks[NUM_KICKS];
	void setUsableKick(int i, bool usable) { usableKicks[i] = usable;};
  bool getUsableKick(int i){return usableKicks[i];};

  CornerKickStrategy(){}
};

struct PassStrategy {
  float timeToConsider;
  float distToConsider;
  float targetOffsetDist;
  float upfieldPenaltyFactor;
  float maxTurnFromBall;
};

struct SetPlay {
  ENUM(Type,
    kickoffDiagonalPass,
    kickoffBuryDeep,
    none
  );

  bool active;
  kicks kickType;
  float desiredGoalBearing;
  float maxBearingError;
  bool requiresTargetPlayer;
  Point2D desiredTargetOffset;
  bool reversible;

  SetPlay():
    active(false)
  {}
};

struct SetPlayStrategy {
  static const unsigned int NUM_SET_PLAYS = 2;

  SetPlay plays[NUM_SET_PLAYS];
  float maxTimeInPlaying;
  float maxDistFromBall;
  float maxTargetDistFromLine;

  SetPlay* getPlayPtr(int i) {
    return plays + i;
  }
};

struct BehaviorParamBlock : public MemoryBlock {
  NO_SCHEMA(BehaviorParamBlock);
  BehaviorParamBlock()
  {
    header.version = 14;
    header.size = sizeof(BehaviorParamBlock);
  }

  KickStrategy mainStrategy;
  CornerKickStrategy cornerStrategy;
  ClusterKickStrategy clusterStrategy;
  PassStrategy passStrategy;
  RoleStrategy roleStrategy;
  SetPlayStrategy setPlayStrategy;
};

#endif 
