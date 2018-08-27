#ifndef JOINT_CALIBRATOR_H
#define JOINT_CALIBRATOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <map>
#include "CalibratorSettings.h"
#include <Eigen/Core>
#include <thread>
#include "JointDataset.h"
#include <functional>

class MemoryFrame;
class MemoryCache;
class ImageParams;
class VisionBlocks;
class ImageProcessor;
class LogViewer;
class RobotCalibration;
class MemoryFrame;

class JCSettings : public CalibratorSettings {
  public:
    JCSettings();
    float boardOffset;
    int corners;
    float oX, oY, oZ, oT;
};

class JointCalibrator {
  public:
    using Corners = std::vector<Eigen::Vector2f>;
    using Dataset = JointDataset;
    using Measurement = JointMeasurement;
    JointCalibrator();
    ~JointCalibrator();
    void takeSamples(LogViewer* log);
    Measurement takeSample(MemoryFrame* frame);
    void start(int iterations, std::function<void()> callback);
    void stop();
    void reset();
    void pause();
    void calibrate(int iterations, std::function<void()> callback);
    const bool& calibrating() const { return calibrating_; }
    bool& calibrating() { return calibrating_; }
    void setMemory(MemoryFrame* memory);
    float error() const { return error_; }
    RobotCalibration convertParams(const std::vector<float>& offsets) const;
    std::vector<float> convertParams(const RobotCalibration& cal) const;
    float evaluate(const Dataset& d, const std::vector<float>& offsets) const;
    float evaluate(const Measurement& m, const std::vector<float>& offsets) const;
    void setCalibration(const RobotCalibration& cal);
    const RobotCalibration& getCalibration() const { return *cal_; }
    Corners findChessboardCorners(unsigned char* image) const;
    Corners findChessboardCorners(cv::Mat& image) const;
    Corners projectChessboardCorners(bool left) const;
    float computeProjectionError(const Corners& icorners, const Corners& pcorners) const;
    bool validateProjection(const Corners& icorners, const Corners& pcorners) const;
    bool& left();
    const bool& left() const;
  private:
    std::vector<int>& jointMap();
    const std::vector<int>& jointMap() const;

    JCSettings settings_;
    std::unique_ptr<ImageParams> params_;
    std::unique_ptr<ImageProcessor> processor_;
    std::unique_ptr<MemoryCache> cache_;
    std::unique_ptr<VisionBlocks> vblocks_;
    std::unique_ptr<MemoryFrame> memory_;
    std::unique_ptr<RobotCalibration> cal_;
    float error_;
    std::unique_ptr<std::thread> thread_;
    bool calibrating_;
    std::string datafile_;
};

#endif
