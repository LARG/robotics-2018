#ifndef VISIONCONSTANTS_H
#define VISIONCONSTANTS_H

#include <math/Geometry.h>

#define MAX_POINTS_PER_LINE 400
#define MAX_CORNERPOINTS 20
#define MAX_LINE_BLOBS 100
#define NUM_GOAL_CANDITATES 5
#define MAX_NUM_ROBOTS 3
#define NUM_GOAL_POINTS 1000
#define MAX_BALL_CANDS    20
#define MAX_ORANGE_BLOBS 75
#define NUM_BODY_EXCL_POINTS 5
#define HEAD_STOP_THRESHOLD 0.1
#define MAX_BLOB_VISIONPOINTS 5000
#define ROBOT_CHEST_HEIGHT 380.0
#define JERSEY_WIDTH 160
#define JERSEY_HEIGHT 160
#define MAX_FOCUS_AREA_COUNT 20
#define MAX_FOCUS_AREA 3000
#define FOVx (60.97 / 180.0 * M_PI)
#define FOVy (47.64 / 180.0 * M_PI)

//const float FOVx = (60.97 / 180 * M_PI); // from new specs
//const float FOVy = (47.64 / 180 * M_PI); // from new specs
//#define FOVx DEG_T_RAD * 60.97 // from new specs
//#define FOVy DEG_T_RAD * 47.64 // from new specs

#endif
