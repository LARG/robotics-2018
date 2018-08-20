#ifndef KICKS_H
#define KICKS_H

#include <string>

// ORDER of kicks does matter, if we're still using strategy.selectFirstValidKick
enum kicks {
  FwdLongLargeGapKick,
  FwdLongSmallGapKick,
  FwdSuperStraightKick,
  FwdLongStraightKick,
  FwdMediumStraightKick,
  FwdPass5Kick,
  FwdPass4Kick,
  FwdPass3Kick,
  FwdPass2Kick,
  FwdShortStraightKick,
  Dribble,
  WalkKickFront,
  WalkKickLeftward,
  WalkKickRightward,
  WalkKickLeftwardSide,
  WalkKickRightwardSide,
  QuickDribble,
  NUM_KICKS
};

const std::string kickNames[] = { 
  "FwdLongLargeGapKick",
  "FwdLongSmallGapKick",
  "FwdSuperStraightKick",
  "FwdLongStraightKick",
  "FwdMediumStraightKick",
  "FwdPass5Kick",
  "FwdPass4Kick",
  "FwdPass3Kick",
  "FwdPass2Kick",
  "FwdShortStraightKick",
  "Dribble",
  "WalkKickFront",
  "WalkKickLeftward",
  "WalkKickRightward",
  "WalkKickLeftwardSide",
  "WalkKickRightwardSide",
  "QuickDribble"
};

enum WalkControl{
  WALK_CONTROL_OFF,
  WALK_CONTROL_SET,
  WALK_CONTROL_DONE
};

// speech text is in cfgkick

#define LEFTLEG 0
#define RIGHTLEG 1

#endif
