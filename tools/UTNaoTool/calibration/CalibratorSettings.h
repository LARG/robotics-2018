#include <opencv2/core/core.hpp>

class CalibratorSettings {
  public:
    enum Pattern {
      CHESSBOARD = 0,
      CIRCLES_GRID = 1,
      ASYMMETRIC_CIRCLES_GRID = 2
    };
    cv::Size imageSize;
    cv::Size boardSize;
    Pattern patternType;
    float squareSize;
    float aspectRatio;
    unsigned int flags;
    CalibratorSettings();
};
