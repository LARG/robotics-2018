/**
 * Walk2014Generator.hpp
 * BH 18 Jan 2014
 */

#pragma once

#include <cmath>
#include "Generator.hpp"
#include "BodyModel.hpp"
#include "types/XYZ_Coord.hpp"
#include "types/ActionCommand.hpp"
#include "utils/Timer.hpp"
#include <motion/RSWalkParameters.h>

class Walk2014Generator : Generator {
  public:
    explicit Walk2014Generator();
    ~Walk2014Generator();
    JointValues makeJoints(ActionCommand::All* request,
                           Odometry* odometry,
                           const SensorValues &sensors,
                           BodyModel &bodyModel,
                           float ballX,
                           float ballY);
    bool isActive();
    bool isStanding();
    ActionCommand::Body active;

    enum Walk2014Option {
      STAND        = 0, // with knees straight and stiffness set to zero to conserve energy and heat generation in motors
      STANDUP      = 1, // process of moving from WALK crouch to STAND
      CROUCH       = 2, // process of transitioning from STAND to WALK
      WALK         = 3,
      READY        = 4, // stand still ready to walk
      KICK         = 5,
      NONE         = 6,
      NUMBER_OF_WALK_OPTIONS
    };

    enum WalkState {
      WALKING        = 0,
      STARTING       = 1,
      STOPPING       = 2,
      NOT_WALKING    = 3,
      NUMBER_OF_WALK_STATES
    };

    Walk2014Option walk2014Option;
    WalkState walkState;
    void readOptions(std::string path); //boost::program_options::variables_map& config);
    void reset();
    void stop();
    friend class WalkEnginePreProcessor;

    private:
      bool exactStepsRequested;

      bool stopping;
      bool stopped;
   
    // time step, timers,
    float dt;
    float t;
    float globalTime;

    float timer;
    float T;                                                // period of half a walk cycle
    float footSwitchT;

    const float z;                                          // zero
    const float PI;

    // Nao H25 V4 dimensions - from utils/body.hpp and converted to meters
    float thigh;                                            // thigh length in meters
    float tibia;                                            // tibia length in meters
    float ankle;                                            // height of ankle above ground

    // Walk 2014 parameters in meters and seconds
    float hiph;                                             // variable vertical distance ground to hip in meters
    float hiph0;                                            // some initial hiph
    float foothL;                                           // meters left foot is lifted off the ground
    float foothR;                                           // meters right foot is lifted off the ground
    float nextFootSwitchT;                                  // next time-point at which to change support foot
    float forward;                                          // Omnidirectional walk forward/backward
    float lastForward;                                      // previous forward value accepted
    float forwardL0, forwardL;                              // variable left foot position wrt standing
    float forwardR0, forwardR;                              // variable right foot position wrt standing
    float leftR;                                            // sideways step in meters for right foot
    float leftL;                                            // sideways step in meters for left  foot
    float left, lastLeft;                                   // Omnidirectional walk left/right
    float turn, lastTurn;                                             // Omnidirectional walk CW / CCW
    float power;                                            // Omnidirectional walk - reserved for kicking
    float bend;
    float speed;
    ActionCommand::Body::Foot foot;                         // is this right?
    bool isFast;
    float stiffness;                                        // global stiffness (poweer to motors)
    float turnRL;                                           // turn variable
    float turnRL0;                                          // turnRL at support foot switch
    float swingAngle;                                       // recovery angle for sideStepping
    bool supportFoothasChanged;                             // Indicates that support foot is deemed to have changed
    float balanceAdjustment;
    float coronalBalanceAdjustment;
    float comOffset;                                        // move in meters of CoM in x-direction when walking to spread weight more evenly over foot

    // Gyro filters
    float filteredGyroX;
    float filteredGyroY;
    float filteredGyroSquaredX;
    float filteredGyroSquaredY;

    // Kicks
    float kickT;
    float kickPhase;
    float rock;
    float kneePitchL, kneePitchR, lastKneePitch;
    float anklePitchL, anklePitchR;
    float lastKickForward;
    float lastSide;
    float lastKickTime;
    float shoulderPitchL;                                   // to swing left  arm while walking / kicking
    float shoulderPitchR;                                   // to swing right arm while walking / kicking
    float shoulderRollL;
    float shoulderRollR;
    float dynamicSide;

    //for odometry updates
    float prevTurn;
    float prevForwardL;
    float prevForwardR;
    float prevLeftL;
    float prevLeftR;

    //time for arm limping
    Timer limp;
    bool armStuck;

    // These are for determining when walk kick is finished
    int num_phase_changes_;
    int phase_change_limit_;
    bool in_walk_kick_;
    bool just_finished_kick_;

    void initialise();

    // Use for iterative inverse kinematics for turning (see documentation BH 2010)
    struct Hpr {
      float Hp;
      float Hr;
    };

    /**
     * Calculates the lean angle given:
     * the commanded left step in meters,
     * time thorough walkStep phase, and total walkStep Phase time
     */
    float leftAngle();

    void updateOdometry(Odometry *odometry, bool isLeftSwingFoot);

    /**
     * returns smooth values in the range 0 to 1 given time progresses from 0 to period
     */
    float parabolicReturn(float); // step function with deadTimeFraction/2 delay
    float parabolicStep(float time, float period, float deadTimeFraction);                  // same as above, except a step up
    float linearStep(float time, float period);                                             // linear increase from 0 to 1
    float interpolateSmooth(float start, float end, float tCurrent, float tEnd);            // sinusoidal interpolation

    /**
     * Sets kick settings when starting the kick
     */
    void prepKick(bool isLeft, BodyModel &bodyModel);

    /**
     * Specifies the joint parameters that form the kick 
     */
    void makeForwardKickJoints(float kickLean, float kickStepH,
                               float shiftPeriod, float &footh, float &forward,
                               float &side, float &kneePitch,
                               float &shoulderRoll, float &anklePitch,
                               float &ballY, ActionCommand::All* request);

    /**
     * Adds kick parameters to final joint values 
     */
    void addKickJoints(JointValues &j);

    // Foot to Body coord transform used to calculate IK foot position and ankle tilts to keep foot in ground plane when turning
    XYZ_Coord mf2b(float Hyp, float Hp, float Hr, float Kp, float Ap,
                   float Ar, float xf, float yf, float zf);
    Hpr hipAngles(float Hyp, float Hp, float Hr, float Kp, float Ap,
                  float Ar, float xf, float yf, float zf, XYZ_Coord e);

    void ellipsoidClampWalk(float &forward, float &left, float &turn, float speed);
    float evaluateWalkVolume(float x, float y, float z);
    void restoreDefaultWalkParameters();

    // GSL parameters
    float com_offset_;
    float forward_change_;
    float left_change_;
    float turn_change_;
    float base_walk_period_;
    float walk_hip_height_;
    float stand_hip_height_;
    float max_forward_;
    float max_left_;
    float max_turn_;
    float base_leg_lift_;
    float arm_swing_;
    float forward_extra_foot_height_;
    float left_extra_foot_height_;
    float start_lift_divisor_;
    float pendulum_height_;
    float max_percent_change_;

    // temp parameters for switching after cycle
    float curr_com_offset_;
    float curr_forward_change_;
    float curr_left_change_;
    float curr_turn_change_;
    float curr_base_walk_period_;
    float curr_walk_hip_height_;
    float curr_fwd_extra_foot_height_;
    float curr_left_extra_foot_height_;
    float curr_stand_hip_height_;
    float curr_max_forward_;
    float curr_max_left_;
    float curr_max_turn_;
    float curr_base_leg_lift_;
    float curr_start_lift_divisor_;
    float curr_arm_swing_;
    float curr_pendulum_height_;

    // Limits on changing parameter values
    float delta_com_offset_;
    float delta_forward_change_;
    float delta_left_change_;
    float delta_turn_change_;
    float delta_base_walk_period_;
    float delta_walk_hip_height_;
    float delta_stand_hip_height_;
    float delta_max_forward_;
    float delta_max_left_;
    float delta_max_turn_;
    float delta_base_leg_lift_;
    float delta_arm_swing_;
    float delta_fwd_extra_foot_height_;
    float delta_left_extra_foot_height_;
    float delta_start_lift_divisor_;
    float delta_pendulum_height_;
};


