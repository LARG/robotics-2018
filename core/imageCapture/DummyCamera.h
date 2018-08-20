#ifndef DUMMYCAMERA_OPYEW0C7
#define DUMMYCAMERA_OPYEW0C7

#include "NaoCamera.h"

class DummyCamera : public NaoCamera {
    public:
  DummyCamera(const ImageParams& iparams, CameraParams& a, CameraParams& b): NaoCamera(iparams, a,b){timeStamp=0;}
  void updateBuffer() {timeStamp++;}
  unsigned char* getImage() const {return NULL;}
  unsigned getTimeStamp() const {return timeStamp;}
  int getControlSetting(unsigned int id) {return 0;}
  bool setControlSetting(unsigned int id, int value) {return true;}
  void setCameraParams() {}
  void getCameraParams() {}
  void reset() {}
  bool selfTest() {return true;}
  int timeStamp;
};

#endif /* end of include guard: DUMMYCAMERA_OPYEW0C7 */
