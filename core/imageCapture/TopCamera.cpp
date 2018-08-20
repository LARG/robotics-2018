#include "TopCamera.h"

TopCamera::TopCamera(const ImageParams& iparams, CameraParams& p, CameraParams& rp) : NaoCamera(iparams, p,rp) {
    device_path_ = "/dev/video0";
    vflip_ = true;
    hflip_ = true;
    init();
}
