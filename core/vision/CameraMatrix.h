#ifndef CAMERA_MATRIX_H
#define CAMERA_MATRIX_H

#include <math/Geometry.h>
#include <vision/VisionConstants.h>
#include <common/RobotInfo.h>
#include <common/ImageParams.h>
#include <common/RobotDimensions.h>
#include <vision/structures/Coordinates.h>
#include <vision/structures/Position.h>
#include <vision/structures/SphericalPosition.h>
#include <common/RobotCalibration.h>
#include <math/Pose3D.h>
#include <Eigen/Core>
#include <Eigen/LU>

/// @ingroup vision
class CameraMatrix {
  private:
    Eigen::Matrix4f worldToCam_, camToWorld_;
    Eigen::Matrix3f coordinateShift_;
    Eigen::Matrix3f cameraCalibration_;
    Eigen::Vector3f cameraPosition_;
    Eigen::Matrix3f cameraRotation_;
    const ImageParams& iparams_;
    const Camera::Type& camera_;
    RobotCalibration cal_;
    float fx_, fy_, scale_, cx_, cy_;

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    CameraMatrix(const ImageParams& iparams, const Camera::Type& type);

    inline float getCameraBearing(int imageX) const {
      return atan((iparams_.width  / 2 - imageX) / (iparams_.width  / (2 * tan(FOVx / 2))));
    }

    inline float getCameraElevation(int imageY) const {
      return atan((iparams_.height / 2 - imageY) / (iparams_.height / (2 * tan(FOVy / 2))));
    }

    inline float getImageX(float cameraBearing) const {
      return (iparams_.width / 2) * (1 - (tan(cameraBearing) / tan(FOVx / 2)));
    }

    inline float getImageY(float cameraElevation) const {
      return (iparams_.height / 2) * (1 - (tan(cameraElevation) / tan(FOVy / 2)));
    }

    inline const ImageParams& getImageParams() const { return iparams_; }

    Position getWorldPositionByDirectDistance(int imageX, int imageY, float distance = 0.0f) const;
    Position getWorldPositionByDirectDistance(Coordinates c, float distance = 0.0f) const;
    Position getWorldPositionByGroundDistance(int imageX, int imageY, float distance = 0.0f) const;
    Position getWorldPositionByGroundDistance(Coordinates c, float distance = 0.0f) const;
    Position getWorldPosition(int imageX, int imageY, float height = 0.0f) const;
    Position getWorldPosition(Coordinates c, float height = 0.0f) const;
    
    Coordinates getImageCoordinates(float x, float y, float z) const;
    Coordinates getImageCoordinates(Eigen::Vector3f worldPosition) const;
    Coordinates undistort(float x, float y) const;
    
    void updateCameraPose(Pose3D& pose);
  
    float groundDistance(const Position& p) const;
    float directDistance(const Position& p) const;
    float elevation(const Position& p) const;
    float bearing(const Position& p) const;
    float getWorldDistanceByWidth(float cameraWidth, float worldWidth) const;
    float getWorldDistanceByHeight(float cameraHeight, float worldHeight) const;
    float getCameraWidthByDistance(float distance, float worldWidth) const;
    float getCameraHeightByDistance(float distance, float worldHeight) const;
    float getWorldHeight(Coordinates top, Coordinates bottom) const;

    void setCalibration(const RobotCalibration& cal);
};

#endif
