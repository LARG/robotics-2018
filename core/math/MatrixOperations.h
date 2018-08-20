#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <iostream>
#include <Eigen/Core>
#include <limits>
#include <algorithm>
#include <numeric>
#define PRINTMAT(x) std::cout << #x << ":\n" << x << "\n";

class MatrixOperations {
  public:
    template<typename T>
    struct Stats { 
      T max = std::numeric_limits<T>::lowest();
      T min = std::numeric_limits<T>::max();
      T mean = 0;
      T stddev = 0; 
    };

    template<typename T, int R, int C>
    inline static Stats<T> ComputeStatistics(const Eigen::Matrix<T, R, C>& M) {
      Stats<T> s;
      s.mean = M.mean();
      float sqsum = std::inner_product(M.data(), M.data() + M.size(), M.data(), 0.0);
      s.stddev = std::sqrt(sqsum / M.size() - s.mean * s.mean);
      s.min = *std::min_element(M.data(), M.data() + M.size());
      s.max = *std::max_element(M.data(), M.data() + M.size());
      return s;
    }

    template<typename T, int R, int C>
    static T ConvertSingleton(Eigen::Matrix<T, R, C> M) {
      static_assert(R == 1, "The input matrix must have one row.");
      static_assert(C == 1, "The input matrix must have one column.");
      return M(0,0);
    }
    
    template<typename T, int R, int C> 
    inline static Eigen::Matrix<T, R, C> SquareRoot(const Eigen::Matrix<T, R, C>& M) {
      Eigen::Matrix<T,R,C> root = CholeskyDecomposition(M);
      return root;
    }

    template<typename T, int R, int C> 
    inline static Eigen::Matrix<T, R, C> SanitizeCovariance(const Eigen::Matrix<T, R, C>& M) {
      auto root = SquareRoot(M);
      auto square = root * root.transpose();
      return square;
    }
    template<typename T, int R, int Step, typename = std::enable_if_t<Step == R - 1>>
    inline static void CholeskyDecomposition_Step(Eigen::Matrix<T, R, R>& A, Eigen::Matrix<T, R, R>& L, int = 0) {
      L(Step,Step) = A(Step,Step) > 0 ? sqrt(A(Step,Step)) : 1;
    }
    template<typename T, int R, int Step, typename = std::enable_if_t<Step < R - 1>>
    inline static void CholeskyDecomposition_Step(Eigen::Matrix<T, R, R>& A, Eigen::Matrix<T, R, R>& L) {
      L(Step,Step) = A(Step,Step) > 0 ? sqrt(A(Step,Step)) : 1;
      L.template block<R-Step-1,1>(Step+1,Step) = A.template block<R-Step-1,1>(Step+1,Step) / L(Step,Step);
      A.template block<R-Step-1,R-Step-1>(Step+1,Step+1) = A.template block<R-Step-1,R-Step-1>(Step+1,Step+1) - L.template block<R-Step-1,1>(Step+1,Step) * (L.template block<R-Step-1,1>(Step+1,Step)).transpose();
      CholeskyDecomposition_Step<T,R,Step+1>(A,L);
    }
    template<typename T, int R>
    inline static Eigen::Matrix<T, R, R> CholeskyDecomposition(Eigen::Matrix<T, R, R> A) {
      Eigen::Matrix<T, R, R> L = Eigen::Matrix<T, R, R>::Identity();
      CholeskyDecomposition_Step<T,R,0>(A,L);
      return L;
    }
    
    template<typename T, int R>
    static Eigen::Matrix<T, R, R> RankUpdate(Eigen::Matrix<T, R, R> L, Eigen::Matrix<T, R, 1> x, T sigma) {
      for(int i = 0; i < R; i++) {
        auto r = sqrt(pow(L(i,i),2) + sigma * pow(x[i],2));
        auto c = r / L(i,i);
        auto s = x[i] / L(i,i);
        L(i,i) = r;
        for(int j = i+1; j < R; j++) {
          L(i,j) = (L(i,j) + sigma * s * x[j]) / c;
          x[j] = c * x[j] - s*L(i,j);
        }
      }
      return L;
    }

    template<typename T, int R,int C>
    static Eigen::Matrix<T, R, R> HouseholderTransform(Eigen::Matrix<T, R, 2*C+R> input) {
      int rows = input.rows();
      int r = input.cols() - rows;
      T sigma;
      T a;
      T b;
      std::vector<T> v(input.cols());
      Eigen::Matrix<T, R, R> result;
      for(int k=rows-1;k>=0;k--){
        sigma=0.0;
        for(int j=0;j<=r+k;j++){
          sigma=sigma+input(k,j)*input(k,j);
        }
        a=sqrtf(sigma);
        sigma=0.0;
        for(int j=0;j<=r+k;j++){
          if(j==r+k){
            v.at(j)=(input(k,j)-a);
          }
          else{
            v.at(j)=(input(k,j));
          }
          sigma=sigma+v.at(j)*v.at(j);
        }
        a=2.0/(sigma+1e-15);
        for(int i=0;i<=k;i++){
          sigma=0.0;
          for(int j=0;j<=r+k;j++){
            sigma=sigma+input(i,j)*v.at(j);
          }
          b=a*sigma;
          for(int j=0;j<=r+k;j++){
            input(i,j)=input(i,j)-b*v.at(j);
          }
        }
      }
      for(int i=0;i<rows;i++){
        result.col(i) = input.col(r + i);
      }
      return result;
    }
};
#endif
