#ifndef CAMERA_CALIBRATION_H
#define CAMERA_CALIBRATION_H

#include <common/Field.h>
#include <common/RobotInfo.h>
#include <common/RobotDimensions.h>
#include <vision/VisionConstants.h>
#include <common/ImageParams.h>
#include <Eigen/Core>

#include <common/YamlConfig.h>

/// @ingroup vision
class RobotCalibration : public YamlConfig {
  public:
    Camera::Type camera;

    bool enabled;
    bool useLeft;

    float dimensionValues_[RobotDimensions::NUM_DIMENSIONS];
    float jointValues_[NUM_JOINTS];
    float sensorValues_[NUM_SENSORS];

    float
      poseX,
      poseY,
      poseZ,
      poseTheta;

    float // Camera Offsets
      topFOVx,
      topFOVy,
      bottomFOVx,
      bottomFOVy;

    float // Distortion Parameters
      k1, k2, k3,
      p1, p2;

    Eigen::VectorXf extrinsicVector() const {
      Eigen::VectorXf v;
      return v;
    }

    void fromExtrinsicVector(const Eigen::VectorXf& /*v*/) {
    }

    RobotCalibration();
    void applyJoints(float* joints);
    void applySensors(float* sensors);
    void applyDimensions(float* dimensions);
    void revertJoints(float* joints);
    void revertSensors(float* sensors);
    void revertDimensions(float* dimensions);

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
#endif
