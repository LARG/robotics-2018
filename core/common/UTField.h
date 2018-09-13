#ifndef _FIELD_H
#define _FIELD_H

#include "WorldObject.h"
#include <stdio.h>

//#ifdef interface_H
//cout << "Field_h";
//#endif


// setup constants for field size, landmark locations, etc

const float BALL_RADIUS = 31;

const float FIELD_Y = 2000;
const float FIELD_X = 3000;
const float GRASS_Y = 2500;
const float GRASS_X = 5000;

const float HALF_FIELD_Y = FIELD_Y/2.0;
const float HALF_FIELD_X = FIELD_X/2.0;
const float HALF_GRASS_Y = GRASS_Y/2.0;
const float HALF_GRASS_X = GRASS_X/2.0;

const float GOAL_Y = 500;
const float GOAL_POST_WIDTH = 70;
const float GOAL_WIDTH = GOAL_Y + GOAL_POST_WIDTH;
const float GOAL_X = 500;
const float HALF_GOAL_Y = GOAL_Y / 2.0;
const float PENALTY_Y = 2200;
const float PENALTY_X =  600;
const float CIRCLE_DIAMETER = 1500;
const float CIRCLE_RADIUS = CIRCLE_DIAMETER / 2.0;
const float LINE_WIDTH = 50;

const float GOAL_HEIGHT = 325;

const float FIELD_CENTER_X = 0;
const float FIELD_CENTER_Y = 0;

const float PENALTY_CROSS_X = HALF_FIELD_X - 1300;
const float PENALTY_MARK_SIZE = 106;

const float CIRCLE_HEX_LENGTH = 2.0*(CIRCLE_RADIUS*sinf(DEG_T_RAD*30.0));

// Some rectangles

const Rectangle FIELD =
Rectangle( Point2D( -FIELD_X / 2,  FIELD_Y / 2 ),
	   Point2D(  FIELD_X / 2, -FIELD_Y / 2 ) );

const Rectangle GRASS =
Rectangle( Point2D( -GRASS_X / 2,  GRASS_Y / 2 ),
	   Point2D(  GRASS_X / 2, -GRASS_Y / 2 ) );

// circle and cross points
const Point2D circleLocation = Point2D(0, 0);
const Point2D oppCrossLocation = Point2D(PENALTY_CROSS_X, 0);
const Point2D ownCrossLocation = Point2D(-PENALTY_CROSS_X, 0);


// Landmark locations
const Point2D landmarkLocation[] = {
  Point2D(0, 0),  // center circle

  Point2D(HALF_FIELD_X, HALF_FIELD_Y),       //  WO_BEACON_BLUE_YELLOW
  Point2D(HALF_FIELD_X, -HALF_FIELD_Y),      //  WO_BEACON_YELLOW_BLUE,
  Point2D(0, HALF_FIELD_Y),                  //  WO_BEACON_BLUE_PINK
  Point2D(0, -HALF_FIELD_Y),                  //  WO_BEACON_PINK_BLUE
  Point2D(-HALF_FIELD_X, HALF_FIELD_Y),      //  WO_BEACON_PINK_YELLOW
  Point2D(-HALF_FIELD_X, -HALF_FIELD_Y),     //  WO_BEACON_YELLOW_PINK,

  Point2D( -FIELD_X / 2, 0),                 // WO_OWN_GOAL
  Point2D( FIELD_X / 2, 0 ),                 // WO_OPP_GOAL

  Point2D( -FIELD_X / 2, -GOAL_Y / 2),       // WO_OWN_LEFT_GOALPOST
  Point2D( FIELD_X / 2, GOAL_Y / 2 ),        // WO_OPP_LEFT_GOALPOST

  Point2D( -FIELD_X / 2, GOAL_Y / 2),        // WO_OWN_RIGHT_GOALPOST
  Point2D( FIELD_X / 2, -GOAL_Y / 2)         // WO_OPP_RIGHT_GOALPOST
};


// Line intersection locations
const Point2D intersectionLocation[] = {
  // L
  Point2D( FIELD_X / 2, FIELD_Y / 2),                 // 0 WO_OPP_FIELD_LEFT_L
  Point2D( FIELD_X / 2, -FIELD_Y / 2),                //   WO_OPP_FIELD_RIGHT_L
  Point2D( FIELD_X / 2 - PENALTY_X, PENALTY_Y / 2),   // 2 WO_OPP_PEN_LEFT_L
  Point2D( FIELD_X / 2 - PENALTY_X, -PENALTY_Y / 2),  //   WO_OPP_PEN_RIGHT_L
  Point2D( -FIELD_X / 2 + PENALTY_X, PENALTY_Y / 2),  // 4 WO_OWN_PEN_RIGHT_L
  Point2D( -FIELD_X / 2 + PENALTY_X, -PENALTY_Y / 2), //   WO_OWN_PEN_LEFT_L
  Point2D( -FIELD_X / 2, FIELD_Y / 2),                // 6 WO_OWN_FIELD_RIGHT_L
  Point2D( -FIELD_X / 2, -FIELD_Y / 2),               // 7 WO_OWN_FIELD_LEFT_L
  
  Point2D(  HALF_FIELD_X + GOAL_X, -HALF_GOAL_Y),      // Back right of opp goal post
  Point2D(  HALF_FIELD_X + GOAL_X,  HALF_GOAL_Y),      // Back left of opp goal post

  Point2D( -HALF_FIELD_X - GOAL_X, -HALF_GOAL_Y),      // Back right of own goal post
  Point2D( -HALF_FIELD_X - GOAL_X,  HALF_GOAL_Y),      // Back left of own goal post

  // T
  Point2D( FIELD_X / 2, PENALTY_Y / 2),               // 8  WO_OPP_PEN_LEFT_T
  Point2D( FIELD_X / 2, -PENALTY_Y / 2),              //    WO_OPP_PEN_RIGHT_T
  Point2D( 0, FIELD_Y / 2),                           // 10 WO_CENTER_TOP_T
  Point2D( 0, -FIELD_Y / 2),                          //    WO_CENTER_BOTTOM_T,
  Point2D( -FIELD_X / 2, PENALTY_Y / 2),              // 12 WO_OWN_PEN_RIGHT_T
  Point2D( -FIELD_X / 2, -PENALTY_Y / 2),             // 13 WO_OWN_PEN_LEFT_T
  
  Point2D(  HALF_FIELD_X, -HALF_GOAL_Y),              // front right of opp goal
  Point2D(  HALF_FIELD_X,  HALF_GOAL_Y),              // front left of opp goal

  Point2D( -HALF_FIELD_X, -HALF_GOAL_Y),              // front right of own goal
  Point2D( -HALF_FIELD_X,  HALF_GOAL_Y)               // front left of own goal
};


// Line location
const Point2D lineLocationStarts[] = {

  // HORIZONTAL LINES
  intersectionLocation[WO_OPP_FIELD_LEFT_L  - INTERSECTION_OFFSET],    // WO_OPP_GOAL_LINE
  intersectionLocation[WO_OPP_PEN_LEFT_L    - INTERSECTION_OFFSET],    // WO_OPP_PENALTY
  intersectionLocation[WO_CENTER_TOP_T      - INTERSECTION_OFFSET],    // WO_CENTER_LINE
  intersectionLocation[WO_OWN_PEN_RIGHT_L   - INTERSECTION_OFFSET],    // WO_OWN_PENALTY
  intersectionLocation[WO_OWN_FIELD_RIGHT_L - INTERSECTION_OFFSET],    // WO_OWN_GOAL_LINE

  intersectionLocation[WO_OPP_BACK_RIGHT_GOAL_L - INTERSECTION_OFFSET],
  intersectionLocation[WO_OWN_BACK_RIGHT_GOAL_L - INTERSECTION_OFFSET],

  // VERTICAL LINES
  intersectionLocation[WO_OPP_FIELD_LEFT_L  - INTERSECTION_OFFSET],    // WO_TOP_SIDE_LINE
  intersectionLocation[WO_OPP_PEN_LEFT_L    - INTERSECTION_OFFSET],    // WO_PENALTY_TOP_OPP
  intersectionLocation[WO_OWN_PEN_RIGHT_L   - INTERSECTION_OFFSET],    // WO_PENALTY_TOP_OWN
  intersectionLocation[WO_OPP_PEN_RIGHT_L   - INTERSECTION_OFFSET],    // WO_PENALTY_BOTTOM_OPP
  intersectionLocation[WO_OWN_PEN_LEFT_L    - INTERSECTION_OFFSET],    // WO_PENALTY_BOTTOM_OWN
  intersectionLocation[WO_OPP_FIELD_RIGHT_L - INTERSECTION_OFFSET],    // WO_BOTTOM_SIDE_LINE

  intersectionLocation[WO_OPP_BACK_LEFT_GOAL_L  - INTERSECTION_OFFSET], // WO_OPP_LEFT_GOALBAR
  intersectionLocation[WO_OPP_BACK_RIGHT_GOAL_L - INTERSECTION_OFFSET], // WO_OPP_RIGHT_GOALBAR
  intersectionLocation[WO_OWN_BACK_LEFT_GOAL_L  - INTERSECTION_OFFSET], // WO_OWN_LEFT_GOALBAR
  intersectionLocation[WO_OWN_BACK_RIGHT_GOAL_L - INTERSECTION_OFFSET]  // WO_OWN_RIGHT_GOALBAR
};

// Line location
const Point2D lineLocationEnds[] = {

  // HORIZONTAL LINES
  intersectionLocation[WO_OPP_FIELD_RIGHT_L - INTERSECTION_OFFSET],    // WO_OPP_GOAL_LINE
  intersectionLocation[WO_OPP_PEN_RIGHT_L   - INTERSECTION_OFFSET],    // WO_OPP_PENALTY
  intersectionLocation[WO_CENTER_BOTTOM_T   - INTERSECTION_OFFSET],    // WO_CENTER_LINE
  intersectionLocation[WO_OWN_PEN_LEFT_L    - INTERSECTION_OFFSET],    // WO_OWN_PENALTY
  intersectionLocation[WO_OWN_FIELD_LEFT_L  - INTERSECTION_OFFSET],    // WO_OWN_GOAL_LINE
  
  intersectionLocation[WO_OPP_BACK_LEFT_GOAL_L - INTERSECTION_OFFSET], // opp back goalbar
  intersectionLocation[WO_OWN_BACK_LEFT_GOAL_L - INTERSECTION_OFFSET], // own back goalbar

  // VERTICAL LINES
  intersectionLocation[WO_OWN_FIELD_RIGHT_L - INTERSECTION_OFFSET],    // WO_TOP_SIDE_LINE
  intersectionLocation[WO_OPP_PEN_LEFT_T    - INTERSECTION_OFFSET],    // WO_PENALTY_TOP_OPP
  intersectionLocation[WO_OWN_PEN_RIGHT_T   - INTERSECTION_OFFSET],    // WO_PENALTY_TOP_OWN
  intersectionLocation[WO_OPP_PEN_RIGHT_T   - INTERSECTION_OFFSET],    // WO_PENALTY_BOTTOM_OPP
  intersectionLocation[WO_OWN_PEN_LEFT_T    - INTERSECTION_OFFSET],    // WO_PENALTY_BOTTOM_OWN
  intersectionLocation[WO_OWN_FIELD_LEFT_L  - INTERSECTION_OFFSET],    // WO_BOTTOM_SIDE_LINE

  intersectionLocation[WO_OPP_FRONT_LEFT_GOAL_T  - INTERSECTION_OFFSET], // WO_OPP_LEFT_GOALBAR
  intersectionLocation[WO_OPP_FRONT_RIGHT_GOAL_T - INTERSECTION_OFFSET], // WO_OPP_RIGHT_GOALBAR
  intersectionLocation[WO_OWN_FRONT_LEFT_GOAL_T  - INTERSECTION_OFFSET], // WO_OWN_LEFT_GOALBAR
  intersectionLocation[WO_OWN_FRONT_RIGHT_GOAL_T - INTERSECTION_OFFSET]  // WO_OWN_RIGHT_GOALBAR
};


#endif
