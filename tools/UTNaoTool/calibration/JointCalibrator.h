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

class JCSettings : public CalibratorSettings {
  public:
    JCSettings();
    float boardOffset;
    int corners;
    float oX, oY, oZ, oT;
};

class JointCalibrator {
  public:
    typedef JointDataset Dataset;
    typedef JointMeasurement Measurement;
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
    void setCalibration(RobotCalibration* cal);
    const RobotCalibration* getCalibration() const { return cal_; }
    std::vector<Eigen::Vector2f> findChessboardCorners(unsigned char* image) const;
    std::vector<Eigen::Vector2f> findChessboardCorners(cv::Mat& image) const;
    std::vector<Eigen::Vector2f> projectChessboardCorners(bool left) const;
    float computeProjectionError(const std::vector<Eigen::Vector2f>& icorners, const std::vector<Eigen::Vector2f>& pcorners) const;
    bool& left();
    const bool& left() const;
  private:
    std::vector<int>& jointMap();
    const std::vector<int>& jointMap() const;

    JCSettings settings_;
    ImageProcessor* processor_;
    ImageParams* params_;
    MemoryCache* cache_;
    VisionBlocks* vblocks_;
    MemoryFrame* memory_;
    RobotCalibration* cal_;
    float error_;
    std::thread* thread_;
    bool calibrating_;
    std::string datafile_;
};

#endif
