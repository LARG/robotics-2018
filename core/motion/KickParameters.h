#ifndef KICKPARAMETERS
#define KICKPARAMETERS

#include <string>
#include <math/Vector3.h>
#include <fstream>

class KickState {
  public:
  enum State {
    STAND,
    SHIFT,
    LIFT,
    ALIGN,
    KICK1,
    KICK2,
    SPLINE,
    RESETFOOT,
    FOOTDOWN,
    SHIFTBACK,
    FINISHSTAND,
    NONE,
    WALK,

    NUM_KICK_STATES = FINISHSTAND+1
  };

  static std::string getName(State state) {
    std::string names[] = {
      "STAND",
      "SHIFT",
      "LIFT",
      "ALIGN",
      "KICK1",
      "KICK2",
      "SPLINE",
      "RESETFOOT",
      "FOOTDOWN",
      "SHIFTBACK",
      "FINISHSTAND",
      "NONE",
      "WALK"
    };
    if ((state < 0) || (state >= NUM_KICK_STATES + 1))
      return "UNKNOWN";
    return names[state];
  }

};


struct KickStateInfo {
  float state_time;
  float joint_time;
  float comOffset;
  float comOffsetRight;
  float comOffsetLeft;
  Vector3<float> com;
  Vector3<float> swing;
};


#define MAX_SPLINE_POINTS 20

struct KickParameters {
  KickParameters():
    num_swing_spline_pts(-1),
    num_stance_spline_pts(-1),
    step_into_kick_(false)
  { }

  // splines
  bool use_akima_spline;
  bool use_stance_spline;

  int num_swing_spline_pts;
  double spline_swing_times[MAX_SPLINE_POINTS];
  double spline_swing_xs[MAX_SPLINE_POINTS];
  double spline_swing_ys[MAX_SPLINE_POINTS];
  double spline_swing_zs[MAX_SPLINE_POINTS];
  
  int num_stance_spline_pts;
  double spline_stance_times[MAX_SPLINE_POINTS];
  double spline_stance_xs[MAX_SPLINE_POINTS];
  double spline_stance_ys[MAX_SPLINE_POINTS];
  double spline_stance_zs[MAX_SPLINE_POINTS];

  //
  float kick_time_;
  float offset_;

  float swing_length_;
  float swing_time_;

  float align_height_;
  float kick_height_;


  // ideal ball placements - i.e. what the kick was developed for
  float ideal_ball_side_left_swing_;
  float ideal_ball_side_right_swing_;

  // used to define angles for actual kick state (spline)
  float l_hip_roll_before_;
  float l_hip_pitch_before_;
  float l_knee_pitch_before_;
  float l_ankle_roll_before_;
  float l_ankle_pitch_before_;
  float l_hip_roll_after_;
  float l_hip_pitch_after_;
  float l_knee_pitch_after_;
  float l_ankle_roll_after_;
  float l_ankle_pitch_after_;

  // bounds on what the kick can physically do
  float inside_side_dist_from_stance_left_swing_;
  float outside_side_dist_from_stance_left_swing_;
  float max_fwd_dist_from_stance_left_swing_;
  float inside_side_dist_from_stance_right_swing_;
  float outside_side_dist_from_stance_right_swing_;
  float max_fwd_dist_from_stance_right_swing_;

  // a place to set robot-specific parameters for the distance functions
  float straight_value_one_left_swing_;
  float straight_value_two_left_swing_;
  float straight_value_one_right_swing_;
  float straight_value_two_right_swing_;

  // do we want to step into the kick?
  bool step_into_kick_;

  KickStateInfo states[KickState::NUM_KICK_STATES];

  KickStateInfo* getStateInfoPtr(KickState::State state) {
    return &(states[state]);
  }
};
#endif
