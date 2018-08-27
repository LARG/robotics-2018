#ifndef IMAGECAPTURE_NQY7O454
#define IMAGECAPTURE_NQY7O454

#include <imageCapture/NaoCamera.h>
#include <imageCapture/BottomCamera.h>
#include <imageCapture/TopCamera.h>

#include <memory/MemoryFrame.h>
#include <memory/CameraBlock.h>
#include <memory/FrameInfoBlock.h>
#include <memory/ImageBlock.h>
#include <common/RobotInfo.h>

#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

class ImageCapture {
public:
  ImageCapture(MemoryFrame *memory);
  void takeV4LPicture();
  void initVision();

  NaoCamera* bottom_camera_;
  NaoCamera* top_camera_;
  void dequeueThread();
  void requeue();

  void enableAutoWB();
  void lockWB();

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

  // Buffer queueing synchronization
  std::mutex buffer_mutex_;
  std::atomic<bool> buffer_dequeued_, buffer_requeued_;
  std::unique_ptr<std::thread> buffer_thread_;
  std::condition_variable dequeue_cv_, requeue_cv_;
};
#endif /* end of include guard: IMAGECAPTURE_NQY7O454 */
