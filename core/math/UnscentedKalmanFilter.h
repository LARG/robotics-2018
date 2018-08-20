#ifndef UNSCENTED_KALMAN_FILTER_H
#define UNSCENTED_KALMAN_FILTER_H

#include <math/PairwiseCostMergeUtility.h>
#include <math/MatrixOperations.h>
#include <functional>
#include <map>
#include <limits>
#include <set>
#include <list>

#define MEAS_BODY \
  MeasVec nonlinearPredictionTransform(const Base::StateVec& state, const Observation& observation); \
  void update(const Observation& observation); \
  TextLogger* tlogger() { return static_cast<Derived&>(ukf_).tlogger(); } \
  const Base::StateVec& state() { return ukf_.state(); }

#define DEFAULT_MEAS_DEFINITION(ClassName, MeasSize) \
    class ClassName : public FilterMeasurement<MeasSize> { \
      public: \
        ClassName(BaseUKF& ukf); \
        MEAS_BODY \
    }

class TextLogger;

template <typename T, int StateSize, typename ObservationType, typename UKFParametersType>
class UnscentedKalmanFilter {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    typedef Eigen::Matrix<T, StateSize, 1> StateVec;
    typedef Eigen::Matrix<T, StateSize, StateSize> StateCov;
    typedef Eigen::Matrix<T, StateSize, 2*StateSize + 1> StateSigma;
    typedef Eigen::Matrix<T, 2*StateSize + 1, 1> WeightVec;
    using Scalar = T;
    using Observation = ObservationType;
    using Params = UKFParametersType;

    template<int MeasSize>
    class FilterMeasurement {
      public:
        typedef Eigen::Matrix<T, MeasSize, 1> MeasVec;
        typedef Eigen::Matrix<T, MeasSize, MeasSize> MeasCov;
        typedef Eigen::Matrix<T, MeasSize, 2*StateSize + 1> MeasSigma;
        typedef Eigen::Matrix<T, StateSize, MeasSize> StateMeasCov;
        typedef FilterMeasurement<MeasSize> BaseMeas;
        typedef UnscentedKalmanFilter<T, StateSize, ObservationType, UKFParametersType> BaseUKF;

        FilterMeasurement(UnscentedKalmanFilter& ukf) : ukf_(ukf), tlogger_(ukf.tlogger()) {
          maxInnovation_ = ukf.params().MAX_INNOVATION_DEFAULT;
        }
        FilterMeasurement(const FilterMeasurement& m) : ukf_(m.ukf_), tlogger_(m.ukf_.tlogger()) { }
        FilterMeasurement& operator=(const FilterMeasurement& m) {
          this->ukf_ = m.ukf_;
        }
        virtual ~FilterMeasurement() = default;

        // Computes a predicted measurement from a hypothetical state and the current observation.
        virtual MeasVec nonlinearPredictionTransform(const StateVec& state, const ObservationType& obs) = 0;

        // Computes a measurement and its uncertainty from an observation.
        virtual void update(const ObservationType& obs) = 0;

        // Updates the UKF based on the provided observation and its associated measurement.
        T baseUpdate(const MeasVec& measurement, const MeasCov& uncertainty, const ObservationType& obs, bool commitOutliers = false, bool enableDegrade = true) { 
          ukf_.attempt();
          ukf_.updateSigmas();
          fmeas_ = MeasVec::Zero();
          yscript_ = MeasSigma::Zero();
          for(int i = 0; i <= 2*StateSize; i++) {
            StateVec v = ukf_.xscript_.col(i);
            MeasVec m = nonlinearPredictionTransform(v, obs);
            yscript_.col(i) = m;
            fmeas_ += ukf_.weights_[i] * m;
          }

          mcov_ = uncertainty * ukf_.params_.GLOBAL_COV_FACTOR;
          for(int i = 0; i <= 2*StateSize; i++) {
            MeasVec col = yscript_.col(i) - fmeas_;
            normalize(col);
            difference(col, yscript_.col(i), fmeas_);
            mcov_ += ukf_.weights_[i] * col * col.transpose();
          }

          smcov_ = StateMeasCov::Zero();
          for(int i = 0; i <= 2*StateSize; i++) {
            StateVec xdiff = ukf_.xscript_.col(i) - ukf_.state_;
            ukf_.normalize(xdiff);
            MeasVec ydiff = yscript_.col(i) - fmeas_;
            normalize(ydiff);
            difference(ydiff, yscript_.col(i), fmeas_);
            smcov_ += ukf_.weights_[i] * xdiff * ydiff.transpose();
          }

          imcov_ = mcov_.inverse();
          diff_ = measurement - fmeas_;
          normalize(diff_);
          difference(diff_, measurement, fmeas_);
          gain_ = smcov_ * imcov_;
          adj_ = gain_ * diff_;
          innovation_ = (diff_.transpose() * imcov_ * diff_)[0];
          if(fabs(innovation_) < maxInnovation_) {
            commitUpdate();
          } else {
            if(enableDegrade) ukf_.alpha_ *= ukf_.params_.OUTLIER_ALPHA_DEGRADE_FACTOR;
            if(commitOutliers) commitUpdate();
            ukf_.throwout();
          }
          return innovation_;
        }

        void commitUpdate() {
          ukf_.state_ = ukf_.state_ + adj_;
          ukf_.normalize(ukf_.state_);
          ukf_.cov_ -= gain_ * mcov_ * gain_.transpose();
          ukf_.cov_ = MatrixOperations::SanitizeCovariance(ukf_.cov_);
        }
      protected:
        std::map<int, std::function<T(T)>> normalizers_;
        std::map<int, std::function<T(T,T)>> differencers_;
        void normalize(MeasVec& v) {
          BaseUKF::normalize<MeasSize>(v, normalizers_);
        };
        void difference(MeasVec& output, const MeasVec& left, const MeasVec& right) {
          for(auto kvp : differencers_) {
            auto idx = kvp.first;
            auto& func = kvp.second;
            output[idx] = func(left[idx], right[idx]);
          }
        }
        UnscentedKalmanFilter& ukf_;
        float maxInnovation_;

        // Results of the latest computation
        MeasVec fmeas_, diff_;
        MeasSigma yscript_;
        StateVec adj_;
        MeasCov mcov_, imcov_;
        StateMeasCov gain_, smcov_;
        float innovation_;
        TextLogger*& tlogger_;
    };
    template <int MeasSize>
    friend class FilterMeasurement;
    
    inline T& alpha() { return alpha_; }
    inline T alpha() const { return alpha_; }
    inline const UKFParametersType& params() const { return params_; }
    inline const StateVec& state() const { return state_; }
    inline const StateCov& covariance() const { return cov_; }
    inline const StateCov& process() const { return process_; }
    inline const StateSigma& xscript() const { return xscript_; }
    inline const WeightVec& weights() const { return weights_; }
    inline TextLogger*& tlogger() { return tlogger_; }
    inline const TextLogger* tlogger() const { return tlogger_; }
    virtual void loadParams(const UKFParametersType& params) { params_ = params; }
    UnscentedKalmanFilter(const UKFParametersType& params) : params_(params) { 
      attempts_ = throwouts_ = 0;
      attempt_history_size_ = throwout_history_size_ = 0;
      alpha_ = 1;
      for(int i = 0; i <= 2*StateSize; i++)
        weights_[i] = 1.0/(2*StateSize + 1);
      state_ = StateVec::Zero();
      cov_ = StateCov::Identity();
      process_ = StateCov::Identity();
    }
    virtual ~UnscentedKalmanFilter() = default;
    void start() { attempts_ = throwouts_ = 0; }
    void finish() {
      if(attempts_ == 0) return;
      attempt_history_.push_back(attempts_);
      attempt_history_size_++;
      if(attempt_history_size_ > params_.THROWOUT_RATE_HISTORY_LENGTH) {
        attempt_history_.pop_front();
        attempt_history_size_--;
      }
      throwout_history_.push_back(throwouts_);
      throwout_history_size_++;
      if(throwout_history_size_ > params_.THROWOUT_RATE_HISTORY_LENGTH) {
        throwout_history_.pop_front();
        throwout_history_size_--;
      }
    }
    void degrade(int n = 1) { alpha_ *= pow(params_.OUTLIER_ALPHA_DEGRADE_FACTOR, n); }
    void throwout(int n = 1) { throwouts_ += n; degrade(n); }
    void attempt(int n = 1) { attempts_ += n; }
    int throwouts() const { 
      int t = 0;
      for(const auto& i : throwout_history_) t += i;
      return t;
    }
    int attempts() const { 
      int a = 0;
      for(const auto& i : attempt_history_) a += i;
      return a;
    }
    float throwoutRate() const { return attempts_ == 0 ? 0.0f : (float)throwouts_ / attempts_; }
    float avgThrowoutRate() const {
      if(attempt_history_size_ == 0 || throwout_history_size_ == 0) return 0.0f;
      int a = attempts(), t = throwouts();
      if(a == 0) return 0.0f;
      return (float)t/a;
    }
  protected:
    // Evolves a particular sigma point according to the process model.
    virtual StateVec nonlinearTimeTransform(const StateVec& state, const ObservationType& obs) = 0;
    
    // Normalizer functions are used for recomputing state variables with cyclic properties
    // or valid ranges. Particularly these are designed to handle angle measurements.
    std::map<int, std::function<T(T)>> normalizers_;
    void normalize(StateVec& v) {
      normalize<StateSize>(v, normalizers_);
    }
    template<int R>
    static void normalize(Eigen::Matrix<T, R, 1>& v, const std::map<int, std::function<T(T)>>& normalizers) {
      for(auto kvp : normalizers) {
        auto idx = kvp.first;
        auto& func = kvp.second;
        v[idx] = func(v[idx]);
      }
    }
    
    // Recomputes sigma points - This function samples deterministically around
    // the current state to account for nonlinearity.
    void updateSigmas() {
      StateCov stddev = MatrixOperations::SquareRoot(cov_);
      xscript_.col(0) = state_;
      for(int i=1; i<=StateSize; i++) {
        auto term = sqrtf(StateSize / (1 - weights_[i])) * stddev.col(i - 1);
        StateVec col = state_ + term;
        xscript_.col(i) = col;
        col = state_ - term;
        xscript_.col(StateSize + i) = col;
      }
    }

    // Evolves the sigma points according to the process model and recomputes the current state 
    // and covariance based on the evolved sigma points.
    void timeUpdate(const ObservationType& obs) {
      updateSigmas();
      state_ = StateVec::Zero();
      for(int i = 0; i <= 2*StateSize; i++) {
        StateVec v = xscript_.col(i);
        v = nonlinearTimeTransform(v, obs);
        xscript_.col(i) = v;
        state_ += weights_[i] * v;
        normalize(state_);
      }
     
      cov_ = process_;
      for(int i = 0; i  <= 2*StateSize; i++) {
        StateVec col = xscript_.col(i) - state_;
        normalize(col);
        cov_ += weights_[i] * col * col.transpose();
      }
    }

    WeightVec weights_;
    StateVec state_;
    StateCov process_;
    StateCov cov_;
    StateSigma xscript_;
    int throwouts_, attempts_;
    std::list<int> attempt_history_, throwout_history_;
    int attempt_history_size_, throwout_history_size_;
    TextLogger* tlogger_;
    
    T alpha_;
    UKFParametersType params_;

  // The following code is used for merging similar models together.
  public:
    virtual UnscentedKalmanFilter* copy() const = 0;
  protected:
    template<typename Derived>
    static Derived* merge_pair(const Derived* left, const Derived* right) {
      auto ukf = left->copy();
      float sumAlpha = ukf->alpha_ + right->alpha();
      ukf->state_ = ukf->state_ * ukf->alpha_ / sumAlpha + right->state() * right->alpha() / sumAlpha;
      ukf->cov_ = ukf->cov_ * ukf->alpha_ / sumAlpha + right->covariance() * right->alpha() / sumAlpha;
      ukf->process_ = ukf->process_ * ukf->alpha_ / sumAlpha + right->process() * right->alpha() / sumAlpha;
      ukf->alpha_ = sumAlpha / 2;
      return static_cast<Derived*>(ukf);
    }
    
    template<typename Derived>
    static std::vector<Derived*> merge(std::vector<Derived*> models, int count, T threshold) {
      static PairwiseCostMergeUtility<Derived,T> merge_utility;
      return merge_utility.merge_items(models, count, threshold,
        &UnscentedKalmanFilter::merge_pair<Derived>,
        &Derived::distance
      );
    }

  public:
    template<typename Derived>
    static std::vector<Derived*> mergeCount(std::vector<Derived*> models, int count) {
      return merge<Derived>(models, count, std::numeric_limits<T>::max());
    }
    template<typename Derived>
    static std::vector<Derived*> mergeThreshold(std::vector<Derived*> models, T threshold) {
      return merge(models, -1, threshold);
    }
};
#endif
