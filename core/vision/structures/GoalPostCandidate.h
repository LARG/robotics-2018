#ifndef GOAL_POST_CANDIDATE_H
#define GOAL_POST_CANDIDATE_H

#include <vision/structures/Position.h>
#include <vision/enums/Colors.h>

/// @ingroup vision
struct GoalPostCandidate {

  // Vision/Localization properties 
  uint16_t xi, xf, yi, yf; 
  uint16_t avgX, avgY;
  Position relPosition;     // relative position in world coordinates (calculated from camera matrix's getWorldPosition())
  
  // Features
  float whitePct;
  float greenBelowPct;
  int edgeSize;
  int edgeStrength;
  float width;              // Estimated world width of the post
  

  int leftEdgeWidth;    // Width of the edge in pixels
  int rightEdgeWidth;   // Width of the edge in pixels

  bool invalid;

};


// FOR GOAL DETECTION
//bool sortBlobPixelRatioPredicate(Blob b1, Blob b2);
bool sortPostEdgeStrengthPredicate(GoalPostCandidate b1, GoalPostCandidate b2);
bool sortPostXiPredicate(GoalPostCandidate b1, GoalPostCandidate b2);

bool overlapping(GoalPostCandidate b1, GoalPostCandidate b2);


#endif
