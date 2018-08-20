#ifndef CAMERA_CALIBRATOR_H
#define CAMERA_CALIBRATOR_H

#include <vision/ImageProcessor.h>
#include <common/RobotCalibration.h>
#include <vision/structures/Sample.h>
#include <math/Geometry.h>
#include <math/Pose2D.h>
#include <vector>
#include <vision/CameraMatrix.h>
/*#include <math/GaussNewtonOptimizer.h>*/
#include <memory/WorldObjectBlock.h>
#include <Eigen/Core>

using namespace std;

class WorldObjectBlock;

typedef Pose2D RobotPose;

class ExtrinsicCalibrator {
  private:
    WorldObjectBlock* world_object_block_;
    vector<LineSegment> getFieldLinesCameraFrame(const RobotCalibration& cal, const CameraMatrix& cmatrix) const;
    vector<LineSegment> getFieldLinesGlobalFrame() const;
    ImageProcessor *processor_;
  public:
    ExtrinsicCalibrator(WorldObjectBlock* world_object_block, ImageProcessor* processor) {
      processor_ = processor;
      world_object_block_ = world_object_block;
    }
    float computeErrorParameterVector(const Sample& sample, const Eigen::VectorXf& parameters);
    static vector<LineSegment> getFieldLinesCameraFrame(const RobotCalibration& cal, WorldObjectBlock* block, const CameraMatrix& cmatrix);

    void calibrate(RobotCalibration& cal,vector<Sample>,int);
    float getSampleErrorCameraFrame(const vector<Sample>& samples, const RobotCalibration& cal) const;
    float getSampleErrorCameraFrame(const Sample&, const RobotCalibration& cal) const;
    float getSampleErrorGlobalFrame(const Sample&, const RobotCalibration& cal) const;
};

#endif
