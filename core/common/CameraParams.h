#ifndef CAMERA_PARAMS_H
#define CAMERA_PARAMS_H

#include <common/Serialization.h>
#include <schema/gen/CameraParams_generated.h>

DECLARE_INTERNAL_SCHEMA(struct CameraParams {
  SCHEMA_METHODS(CameraParams);
  SCHEMA_FIELD(int kCameraAutoWhiteBalance);
  SCHEMA_FIELD(int kCameraExposureAuto);
  SCHEMA_FIELD(int kCameraBacklightCompensation);
  SCHEMA_FIELD(int kCameraBrightness);
  SCHEMA_FIELD(int kCameraContrast);
  SCHEMA_FIELD(int kCameraSaturation);
  SCHEMA_FIELD(int kCameraHue);
  SCHEMA_FIELD(int kCameraExposure);
  SCHEMA_FIELD(int kCameraGain);
  SCHEMA_FIELD(int kCameraSharpness);
});
#endif
