#ifndef BOTTOM_CAMERA_H
#define BOTTOM_CAMERA_H

#include "NaoCamera.h"

class BottomCamera : public NaoCamera {
    public:
  BottomCamera(const ImageParams& iparams, CameraParams&, CameraParams&);
};

#endif
