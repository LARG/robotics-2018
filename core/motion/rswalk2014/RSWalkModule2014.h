#pragma once

#include <motion/WalkModule.h>
#include <common/RobotInfo.h>
#include <common/WorldObject.h>
#include <memory/MemoryFrame.h>
#include <common/InterfaceInfo.h>
#include <memory/MemoryFrame.h>
#include <memory/WalkRequestBlock.h>
#include "WalkEnginePreProcessor.hpp"
#include "ClippedGenerator.hpp"
#include "BodyModel.hpp"
#include "types/Odometry.hpp"
#include "perception/kinematics/Kinematics.hpp"
#include "types/JointValues.hpp"



class WalkingEngine;
class Pose2D;
class BodyModelBlock;
class FrameInfoBlock;
class GameStateBlock;
class JointBlock;
class JointCommandBlock;
class KickRequestBlock;
class OdometryBlock;
class RobotInfoBlock;
class SensorBlock;
class WalkInfoBlock;
class WalkParamBlock;
class WalkRequestBlock;
class SpeechBlock;
class RobotStateBlock;


class RSWalkModule2014: public WalkModule {

  public:
    RSWalkModule2014();
    ~RSWalkModule2014();

    void specifyMemoryDependency();
    void specifyMemoryBlocks();
    void initSpecificModule();

  	// void processWalkRequest(ActionCommand::Body &body);
    void processFrame();
  	void readOptions(std::string path);
    void handleStepIntoKick();

  private:
    void selectivelySendStiffness();
    bool readyToStartKickAfterStep();
    bool updateGyroScopeCalibration(float gyroX, float gyroY);
    bool isRequestForWalk(WalkRequestBlock::Motion motion);
    void setBodyForKick(ActionCommand::Body &body, float &ballX, float &ballY);
    void toStandRequest(ActionCommand::Body &body, bool use_straight_stand);

  	// MemoryFrame blocks
    FrameInfoBlock *frame_info_;
    JointBlock *raw_joints_;
    JointBlock *joints_;
    JointCommandBlock *commands_;
    KickRequestBlock *kick_request_;
    OdometryBlock *odometry_;
    RobotInfoBlock *robot_info_;
    SensorBlock *sensors_;
    WalkInfoBlock *walk_info_;
    WalkParamBlock *walk_params_;
    WalkRequestBlock *walk_request_;
    BodyModelBlock* body_model_;
    SpeechBlock *speech_;
    GameStateBlock* game_state_;
    RobotStateBlock *robot_state_;

    // Walk member variables
    ClippedGenerator* clipper;
    WalkEnginePreProcessor* generator;
    BodyModel bodyModel;
    Kinematics kinematics;
    Odometry odometry_disp;
    bool wasKicking;

    volatile bool standing;
    WalkRequestBlock::Motion prev_command_;

    float slow_stand_start;
    float slow_stand_end;
    float walk_requested_start_time;

    int utJointToRSJoint[NUM_JOINTS];
    int utSensorToRSSensor[NUM_SENSORS];	

    float kick_distance_;
    float kick_angle_;
    bool prev_kick_active_;

    Joints armStart;

    enum StepIntoKickState {
        PERFORMING,
        FINISHED_WITH_STEP,
        NONE
    };

    StepIntoKickState step_into_kick_state_;
    float time_step_into_kick_finished_;

    // Target walk
    float x_target;
    float y_target;
    float angle_target;
    bool target_walk_active_;
    ActionCommand::Body::Foot kick_foot;

    // gyroscopes calibration
    double window_size = 0.001; // 1/# of frames, this needs tuning
    // threshold for calibration: if change of gyro reading over last n frames is small enough
    // note that this is rad/sec, not rad/frame     
    double delta_threshold = 1.0; // this needs tuning
    double reset = 10.0;

    double avg_gyroX = 0.0;
    double avg_delta_gyroX = 10.0; // this influence calibration speed at the first time when start motion
    double offsetX = 0.0;
    double last_gyroX = 0.0;
    float last_gyroX_time;
    int calX_count = 0; //number of calibration performed
      
    double avg_gyroY = 0.0;
    double avg_delta_gyroY = 10.0; 
    double offsetY = 0.0;	
    double last_gyroY = 0.0;
    float last_gyroY_time;
    int calY_count = 0;

    double avg_gyroZ = 0.0;
    double avg_delta_gyroZ = 10.0; 
    double offsetZ = 0.0;	
    double last_gyroZ;
    float last_gyroZ_time;
    int calZ_count = 0;

    double calibration_write_time = -1.0;
    double last_calibration_write = -1.0;

};
