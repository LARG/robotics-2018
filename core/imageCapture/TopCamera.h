#ifndef TOP_CAMERA_H
#define TOP_CAMERA_H
#include "NaoCamera.h"

class TopCamera : public NaoCamera {
    public:
  TopCamera(const ImageParams& iparams, CameraParams&, CameraParams&);
};

#endif
