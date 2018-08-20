#ifndef SENSORBLOCK_
#define SENSORBLOCK_

#include <iostream>
#include <common/RobotInfo.h>
#include <memory/MemoryBlock.h>
#include <schema/gen/SensorBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct SensorBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(SensorBlock);
    SensorBlock() {
      header.version = 4;
      header.size = sizeof(SensorBlock);

      // initialize things
      for (int i = 0; i < NUM_SENSORS; i++){
        values_[i] = 0;
      }

      for (int i = 0; i < NUM_JOINTS; i++){
        joint_temperatures_[i] = 0;
      }

      fsr_feet_ = 0;
      fsr_left_side_ = 0;
      fsr_left_front_ = 0;
      fsr_right_side_ = 0;
      fsr_right_front_ = 0;
    }

    float getJointTemperature(int i){
      return joint_temperatures_[i];
    }

    SCHEMA_FIELD(std::array<float,NUM_SENSORS> prevValues_);
    SCHEMA_FIELD(std::array<float,NUM_SENSORS> values_);
    SCHEMA_FIELD(std::array<float,NUM_SONAR_VALS> sonar_left_);
    SCHEMA_FIELD(std::array<float,NUM_SONAR_VALS> sonar_right_);
    SCHEMA_FIELD(std::array<float,NUM_JOINTS> joint_temperatures_);
    
    SCHEMA_FIELD(float angleXVel);
    SCHEMA_FIELD(float angleYVel);
    SCHEMA_FIELD(float angleZVel);
    SCHEMA_FIELD(float futureRoll);
    SCHEMA_FIELD(float futureTilt);

    SCHEMA_FIELD(float fsr_feet_); // positive means left foot has more weight
    SCHEMA_FIELD(float fsr_left_side_); // positive means left side of left foot has more weight
    SCHEMA_FIELD(float fsr_left_front_); // positive means front side of left foot has more weight
    SCHEMA_FIELD(float fsr_right_side_); // positive means left side of right foot has more weight
    SCHEMA_FIELD(float fsr_right_front_); // positive means front side of right foot has more weight

    float getValue(int index) const { return values_[index]; };
    float getDelta(int index) const { return values_[index] - prevValues_[index]; };
    void copyPrevValues() { for(int i = 0; i < NUM_SENSORS; i++) prevValues_[i] = values_[i]; }
});

#endif 
