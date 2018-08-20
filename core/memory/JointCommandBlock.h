#ifndef JOINT_COMMAND_BLOCK_
#define JOINT_COMMAND_BLOCK_

#include <iostream>
#include <common/RobotInfo.h>
#include <memory/MemoryBlock.h>
#include <schema/gen/JointCommandBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct JointCommandBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(JointCommandBlock);
    JointCommandBlock()  {
      header.version = 7;
      header.size = sizeof(JointCommandBlock);
      body_angle_time_ = 1000;
      head_pitch_angle_time_ = 1000;
      head_yaw_angle_time_ = 1000;
      stiffness_time_ = 1000;
      for (int i=0; i<NUM_JOINTS; i++) {
        angles_[i] = 0;
        stiffness_[i] = 0;
      }
      send_sonar_command_ = false;
      send_body_angles_ = false;
      send_arm_angles_ = false;
      send_head_pitch_angle_ = false;
      send_head_yaw_angle_ = false;
      send_stiffness_ = false;
      sonar_command_ = 0.f;
      send_back_standup_ = false;

      head_pitch_angle_change_ = false;
      head_yaw_angle_change_ = false;
    }

    void setSendAllAngles(bool send, float angle_time = 1000.0f) {
      send_body_angles_ = send;
      send_head_pitch_angle_ = send;
      send_head_yaw_angle_ = send;

      body_angle_time_ = angle_time;
      head_pitch_angle_time_ = angle_time;
      head_yaw_angle_time_ = angle_time;

      head_pitch_angle_change_ = false;
      head_yaw_angle_change_ = false;
    }

    void setPoseRad(const float* src) {
      for (int i=0; i<NUM_JOINTS; i++) {
        angles_[i]=src[i];
      }
    }

    void setPoseDeg(const float* src) {
      for (int i=0; i<NUM_JOINTS; i++) angles_[i]=DEG_T_RAD*src[i];
    }

    void setJointCommand(int i, float val){
      angles_[i] = val;
    }
    
    void setJointCommandDeg(int i, float val){
      angles_[i] = DEG_T_RAD * val;
    }

    void setJointStiffness(int i, float val){
      stiffness_[i] = val;
    }

    void setHeadPan(float target, float time, bool is_change, float stiff=1.0) {
      send_head_yaw_angle_ = true;
      angles_[HeadYaw] = target;
      head_yaw_angle_time_ = time;
      head_yaw_angle_change_ = is_change;
      stiffness_[HeadYaw] = stiff;
    }

    void setHeadTilt(float target, float time, bool is_change, float stiff=1.0){
      send_head_pitch_angle_ = true;
      angles_[HeadPitch] = target;
      head_pitch_angle_time_ = time;
      head_pitch_angle_change_ = is_change;
      stiffness_[HeadPitch] = stiff;
    }

    void setAllStiffness(float target, float time) {
      send_stiffness_ = true;
      stiffness_time_ = time;
      for (int i = 0; i < NUM_JOINTS; i++)
        stiffness_[i] = target;
    }

    // head stuff (separate for yaw and pitch)
    SCHEMA_FIELD(bool send_head_pitch_angle_);
    SCHEMA_FIELD(bool send_head_yaw_angle_);
    SCHEMA_FIELD(bool head_pitch_angle_change_);
    SCHEMA_FIELD(bool head_yaw_angle_change_);
    SCHEMA_FIELD(float head_pitch_angle_time_);
    SCHEMA_FIELD(float head_yaw_angle_time_);

    // body stuff
    SCHEMA_FIELD(bool send_body_angles_);
    SCHEMA_FIELD(bool send_stiffness_);

    SCHEMA_FIELD(bool send_arm_angles_);
    SCHEMA_FIELD(float arm_command_time_);

    SCHEMA_FIELD(std::array<float,NUM_JOINTS> angles_);
    SCHEMA_FIELD(std::array<float,NUM_JOINTS> stiffness_);

    SCHEMA_FIELD(float body_angle_time_);
    SCHEMA_FIELD(float stiffness_time_);

    SCHEMA_FIELD(bool send_back_standup_);

    // sonar
    SCHEMA_FIELD(bool send_sonar_command_);
    SCHEMA_FIELD(float sonar_command_);
});

#endif 
