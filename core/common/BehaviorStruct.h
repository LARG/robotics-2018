#ifndef BEHAVIOR_STRUCT
#define BEHAVIOR_STRUCT

#include <common/Serialization.h>
#include <schema/gen/BehaviorStruct_generated.h>
#include <common/Bits.h>


DECLARE_INTERNAL_SCHEMA(struct BehaviorStruct {
  SCHEMA_METHODS(BehaviorStruct);
  SCHEMA_FIELD(float whistleSd);
  SCHEMA_FIELD(float whistleScore);
  SCHEMA_FIELD(uint16_t ballBid = 3000);
  SCHEMA_FIELD(uint16_t ballMissed = 1000);
  SCHEMA_FIELD(int16_t targetX = 0);
  SCHEMA_FIELD(int16_t targetY = 0);
  SCHEMA_FIELD(uint8_t state = 1);
  SCHEMA_FIELD(uint8_t role = 2);
  SCHEMA_FIELD(uint8_t setPlayType);
  SCHEMA_FIELD(uint8_t setPlayTargetPlayer);
  enum BitIndex {
    BallSeen,
    Fallen,
    SetPlayReversed,
    NumBits
  };
  SCHEMA_FIELD(uint8_t bits = 0);

  inline bool ballSeen() const { return GET_BIT(bits, BallSeen); }
  inline void setBallSeen(bool seen) { SET_BIT(bits, seen, BallSeen); }
  inline bool fallen() const { return GET_BIT(bits, Fallen); }
  inline void setFallen(bool fallen) { SET_BIT(bits, fallen, Fallen); }
  inline bool reversed() const { return GET_BIT(bits, SetPlayReversed); }
  inline void setReversed(bool reversed) { SET_BIT(bits, reversed, SetPlayReversed); }
});
#endif
