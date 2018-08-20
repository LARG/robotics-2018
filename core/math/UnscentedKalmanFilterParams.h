#ifndef UNSCENTED_KALMAN_FILTER_PARAMS_H
#define UNSCENTED_KALMAN_FILTER_PARAMS_H

class UnscentedKalmanFilterParams {
  public:
    float MAX_INNOVATION_DEFAULT = 5.0f;
    float OUTLIER_ALPHA_DEGRADE_FACTOR = 0.9f;
    float GLOBAL_COV_FACTOR = 1.0f;
    int THROWOUT_RATE_HISTORY_LENGTH = 30;
    bool LOADED = false;
};

#endif
