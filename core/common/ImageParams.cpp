#include <common/ImageParams.h>

ImageParams ImageParams::Empty;

ImageParams::ImageParams() { }

ImageParams::ImageParams(Camera::Type camera) {
  if(camera == Camera::TOP) {
    width = 320;
    height = 240;
    defaultHorizontalStepScale = 3;
    defaultVerticalStepScale = 2;
  }
  else {
    width = 320;        // 320
    height = 240;       // 240
    defaultHorizontalStepScale = 1;         // 0
    defaultVerticalStepScale = 1;           // 0
  }

  // Original Parameters
  //width = 640;
  //height = 480;
  //defaultHorizontalStepScale = 2;
  //defaultVerticalStepScale = 1;

  size = width * height;
  rawSize = size * 2;
  factor = width / 160;
  origFactor = width / 640.0f;
}
