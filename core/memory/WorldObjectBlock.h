#ifndef WORLDOJBECTBLOCK_
#define WORLDOJBECTBLOCK_

#include <common/WorldObject.h>
#include <common/States.h>
#include <common/Field.h>
#include <memory/MemoryBlock.h>
#include <schema/gen/WorldObjectBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct WorldObjectBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(WorldObjectBlock);
    WorldObjectBlock() {
      header.version = 3;
      header.size = sizeof(WorldObjectBlock);
    }

    void init(int = 0) {
      // Set the type for each object
      for (int i = 0; i < NUM_WORLD_OBJS; i++) {
        objects_[i].type=(WorldObjectType)i;
        objects_[i].reset();
      }

      // set landmark locations
      for (int i = 0; i < NUM_LANDMARKS; i++) {
        WorldObject *wo = &(objects_[i + LANDMARK_OFFSET]);
        wo->loc = landmarkLocation[i];

        // set heights
        if (wo->isGoal()){
          wo->upperHeight = GOAL_HEIGHT;
          wo->lowerHeight = 0;
          wo->elevation = (wo->upperHeight + wo->lowerHeight) / 2;
        } else {
          wo->upperHeight = 0;
          wo->lowerHeight = 0;
          wo->elevation = 0;
        }
      }

      // set intersection locations
      for (int i = 0; i < NUM_INTERSECTIONS; i++){
        WorldObject *wo = &(objects_[i + INTERSECTION_OFFSET]);
        wo->loc = intersectionLocation[i];

        // set heights
        wo->upperHeight = 0;
        wo->lowerHeight = 0;
        wo->elevation = 0;
      }

      // set line locations
      for (int i = 0; i < NUM_LINES; i++){
        WorldObject *wo = &(objects_[i + LINE_OFFSET]);
        wo->loc = lineLocationStarts[i];
        wo->endLoc = lineLocationEnds[i];
        wo->lineLoc = LineSegment(wo->loc, wo->endLoc);

        // set heights
        wo->upperHeight = 0;
        wo->lowerHeight = 0;
        wo->elevation = 0;
      }

      // set penalty cross locations
      for (int i = 0; i < NUM_CROSSES; i++){
        WorldObject* wo = &(objects_[i + CROSS_OFFSET]);
        Point2D loc = oppCrossLocation;
        if ((i+CROSS_OFFSET) == WO_OWN_PENALTY_CROSS)
          loc = ownCrossLocation;

        wo->loc = loc;
        wo->upperHeight = 0;
        wo->lowerHeight = 0;
        wo->elevation = 0;
      }
    }

    void resetFrames() {
      for (int i = 0; i < NUM_WORLD_OBJS; i++) {
        objects_[i].frameLastSeen = 0;
      }
    }

    /** Resets all world objects. This mainly sets them to be not seen.*/
    void reset() {
      for (int i = 0; i < NUM_WORLD_OBJS; i++) {
        objects_[i].reset();
      }
    }

    WorldObject getObj(unsigned int ind) {
      return objects_[ind];
    }

    WorldObject* getObjPtr(unsigned int ind) {
      return &(objects_[ind]);
    }

    SCHEMA_FIELD(std::array<WorldObject,NUM_WORLD_OBJS> objects_);
});



#endif
