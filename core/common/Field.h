#pragma once

#include <common/WorldObject.h>
#include <vector>

extern const float BALL_RADIUS;

extern const float FIELD_Y;
extern const float FIELD_X;
extern const float GRASS_Y;
extern const float GRASS_X;

extern const float HALF_FIELD_Y;
extern const float HALF_FIELD_X;
extern const float HALF_GRASS_Y;
extern const float HALF_GRASS_X;

extern const float GOAL_Y;
extern const float GOAL_POST_WIDTH;
extern const float GOAL_WIDTH;
extern const float GOAL_X;
extern const float HALF_GOAL_Y;
extern const float PENALTY_Y;
extern const float PENALTY_X;
extern const float CIRCLE_DIAMETER;
extern const float CIRCLE_RADIUS;
extern const float LINE_WIDTH;
extern const float BORDER_STRIP_WIDTH;
extern const float GOAL_HEIGHT;

extern const float FIELD_CENTER_X;
extern const float FIELD_CENTER_Y;

extern const float PENALTY_CROSS_X;
extern const float PENALTY_MARK_SIZE;

extern const float CIRCLE_HEX_LENGTH;

extern const Rectangle FIELD;
extern const Rectangle GRASS;

extern const Point2D circleLocation;
extern const Point2D oppCrossLocation;
extern const Point2D ownCrossLocation;

// Landmark locations
extern const std::vector<Point2D> landmarkLocation;

// Line intersection locations
extern const std::vector<Point2D> intersectionLocation;

// Line location start
extern const std::vector<Point2D> lineLocationStarts;

// Line location end
extern const std::vector<Point2D> lineLocationEnds;
