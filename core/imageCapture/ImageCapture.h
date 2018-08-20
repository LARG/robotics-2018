#ifndef IMAGECAPTURE_NQY7O454
#define IMAGECAPTURE_NQY7O454

#include <pthread.h>
#include "NaoCamera.h"
#include "BottomCamera.h"
#include "TopCamera.h"

#include <memory/MemoryFrame.h>
#include <memory/CameraBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/ImageBlock.h>
#include <common/RobotInfo.h>

// for threading
void* threadedTakeImage(void *arg);

class ImageCapture {
public:
  ImageCapture(MemoryFrame *memory);
  void takeV4LPicture();
  void initVision();

  NaoCamera* bottom_camera_;
  NaoCamera* top_camera_;
private:
  // vision stuff
  pthread_t image_thread_;
  double getSystemTime();
  void checkCameraParams();
  void resetCamera(NaoCamera*);
  void testCameras();

  CameraBlock *camera_info_;
  FrameInfoBlock *vision_frame_info_;
  ImageBlock *image_;

  MemoryFrame *memory_;
  Lock *vision_lock_;

  bool bottom_params_loaded_, top_params_loaded_;
  ImageParams topImageParams_, bottomImageParams_;
};

#endif /* end of include guard: IMAGECAPTURE_NQY7O454 */
