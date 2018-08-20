#ifndef ROBOT_CANDIDATE_H
#define ROBOT_CANDIDATE_H

#include <vision/structures/Position.h>
#include <vision/structures/Blob.h>
#include <vision/enums/Colors.h>

/// @ingroup vision
struct RobotCandidate {

  uint16_t xi, xf, yi, yf;
  uint16_t avgX, avgY;
  
  
  
  // HoughRobotDetector features
  
  int numLines;
  bool used;  
  
  
  
// JerseyRobotDetector features  
  float centerX;
  float centerY;
  float chestButtonHeight;
  float width, height;
  float widthDistance, heightDistance, kinematicsDistance;
  float jerseyColorPercent, greenWhitePercent, whitePercent, correctPercent;
  float worldHeight;
  int feetY, feetX;
  float confidence;
  bool feetMissing;
  Blob* blob;
  Color color;

  Position relTorso, relFeet;
  
  Position relPosition;
  Position absPosition;

  RobotCandidate() : feetMissing(false), blob(NULL), numLines(0), used(false) { }

  static bool sortPredicate(const RobotCandidate& left, const RobotCandidate& right);
};

#endif
