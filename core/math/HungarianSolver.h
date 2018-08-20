#include <Eigen/Core>
#include <vector>

class HungarianSolver {
  public:
    enum Method {
      Reshape,
      Augment
    };
    HungarianSolver(float maxCost) : maxCost_(maxCost) { }
    Eigen::VectorXi solve(const Eigen::MatrixXf& costs, Method method = Augment);
    inline int count() const { return count_; }
  private:
    struct CostItem {
      int col;
      float cost;
    };
    typedef std::vector<CostItem> CostRow;

    Eigen::MatrixXf reshape(const Eigen::MatrixXf& costs);
    Eigen::MatrixXf augment(const Eigen::MatrixXf& costs);
    float maxValue_;
    std::vector<int> colMap_;
    int count_;
    float maxCost_;
};
