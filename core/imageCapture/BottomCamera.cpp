#include "BottomCamera.h"

BottomCamera::BottomCamera(const ImageParams& iparams, CameraParams& p, CameraParams& rp) : NaoCamera(iparams, p,rp) {
    device_path_ = "/dev/video1";
    vflip_ = false;
    hflip_ = false;
    init();
}
