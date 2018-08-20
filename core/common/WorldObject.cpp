#include "WorldObject.h"

// Create the array of objects
//WorldObject worldObjects[NUM_WORLD_OBJS];

WorldObject::WorldObject() {
  seen = false;

  frameLastSeen = -1;
  type = WO_INVALID;
  visionDistance=0.0;
  visionElevation=0.0;
  visionBearing=0.0;
  distance=0.0;
  elevation=0.0;
  bearing=0.0;
  relVel=Point2D(0.0, 0.0);
  loc = Point2D(0, 0);
  height = 0;
  lineLoc = LineSegment(loc, loc);
  orientation=0.0;
  sd= Point2D(0.0, 0.0);
  sdOrientation=0.0;
  absVel= Point2D(0.0, 0.0);
  imageCenterX=0;
  imageCenterY=0;
  fieldLineIndex = -1;
  fromTopCamera = false;
}

WorldObject::WorldObject(WorldObjectType type) : WorldObject() {
  this->type = type;
}

WorldObject::WorldObject(int type) : WorldObject(static_cast<WorldObjectType>( type)) { 
}

WorldObject::~WorldObject() {
}

void WorldObject::reset(){
  seen = false;
  fieldLineIndex = -1;
}
