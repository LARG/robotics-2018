#include "ImageCapture.h"
#include "TopCamera.h"
#include "BottomCamera.h"
#include "DummyCamera.h"

// for threading, not in class
void* threadedTakeImage(void *arg) {
  std::cout << "Starting Take Image thread" << std::endl << std::flush;
  ImageCapture* image_capture = reinterpret_cast<ImageCapture*>(arg);

  while (true) {
    image_capture->takeV4LPicture();
  }
  return NULL;
}

ImageCapture::ImageCapture(MemoryFrame *memory):
  memory_(memory), top_params_loaded_(false), bottom_params_loaded_(false), topImageParams_(Camera::TOP), bottomImageParams_(Camera::BOTTOM)
{
  memory_->getOrAddBlockByName(vision_frame_info_,"raw_vision_frame_info",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(image_,"raw_image",MemoryOwner::IMAGE_CAPTURE);
  memory_->getOrAddBlockByName(camera_info_,"raw_camera_info",MemoryOwner::IMAGE_CAPTURE);

  FrameInfoBlock *motion_frame_info;
  memory_->getBlockByName(motion_frame_info,"frame_info",true,MemoryOwner::INTERFACE);

  camera_info_->cameras_tested_ = false;
  vision_frame_info_->source = MEMORY_ROBOT;
  vision_frame_info_->start_time = motion_frame_info->start_time;

  std::cout << "ImageCapture: Vision Start Time: " << vision_frame_info_->start_time << std::endl;
}

void ImageCapture::initVision() {
  std::cout << "ImageCapture: Initializing Vision Interface" << std::endl << std::flush;

  cleanLock(Lock::getLockName(memory_,LOCK_VISION));
  vision_lock_ = new Lock(Lock::getLockName(memory_,LOCK_VISION));

  std::cout << "ImageCapture: Creating Top V4L2 camera" << std::endl << std::flush;

  std::cout << "ImageCapture: Creating Bottom V4L2 camera" << std::endl << std::flush;
#ifdef COMPILE_FOR_GEODE
  top_camera_ = new DummyCamera(topImageParams_, camera_info_->params_top_camera_,camera_info_->read_params_top_camera_);
  bottom_camera_ = new DummyCamera(bottomImageParams_, camera_info_->params_bottom_camera_,camera_info_->read_params_bottom_camera_);
#else
  top_camera_ = new TopCamera(topImageParams_, camera_info_->params_top_camera_,camera_info_->read_params_top_camera_);
  bottom_camera_ = new BottomCamera(bottomImageParams_, camera_info_->params_bottom_camera_,camera_info_->read_params_bottom_camera_);
#endif

  // std::cout << "ImageCapture(INFO):Sleeping for 5 seconds to allow for camera initialization" << std::endl;
  // sleep(5);

  std::cout << "ImageCapture: Done creating V4L2 cameras" << std::endl << std::flush;

  std::cout << "ImageCapture: Creating thread to take images" << std::endl << std::flush;
  pthread_create(&image_thread_, NULL, (threadedTakeImage), this );
  std::cout << "ImageCapture: Done creating thread to take images" << std::endl << std::flush;

  std::cout << "ImageCapture: Done initializing Vision Interface" << std::endl << std::flush;
}

void ImageCapture::testCameras() {

  int maxAttempts = 100;
  int i = 0;

  for(i = 0; i < maxAttempts; i++)
    if(top_camera_->selfTest()) {
      std::cout << "ImageCapture: Top camera passed self-test\n";
      break;
    }
    else {
      std::cerr << "ImageCapture: Top camera failed self test, resetting...\n";
      delete top_camera_;
      top_camera_ = new TopCamera(topImageParams_, camera_info_->params_top_camera_,camera_info_->read_params_top_camera_);
      top_camera_->updateBuffer();
    }
 

  for(i = 0; i < maxAttempts; i++)
    if(bottom_camera_->selfTest()){
      std::cout << "ImageCapture: Bottom camera passed self-test\n";
      break;
    }
    else {
      std::cerr << "ImageCapture: Bottom camera failed self test, resetting...\n";
      delete bottom_camera_;
      bottom_camera_ = new BottomCamera(bottomImageParams_, camera_info_->params_bottom_camera_,camera_info_->read_params_bottom_camera_);
      bottom_camera_->updateBuffer();
    }
 

  if(i == maxAttempts) {
    std::cerr << "ImageCapture: ERROR: One of the cameras could not be started.";
  }
}

void ImageCapture::takeV4LPicture(){
  vision_lock_->lock();
  checkCameraParams();
  vision_lock_->unlock();

  top_camera_->updateBuffer();
  bottom_camera_->updateBuffer();
  vision_lock_->lock();
  image_->setImgTop(top_camera_->getImage());
  image_->setImgBottom(bottom_camera_->getImage());
  image_->setLoaded();

  vision_frame_info_->frame_id = top_camera_->getTimeStamp();
  vision_frame_info_->seconds_since_start = getSystemTime() - vision_frame_info_->start_time;

  vision_lock_->unlock();
  vision_lock_->notify_one();
}

void ImageCapture::checkCameraParams() {
  if (camera_info_->set_top_params_) {
    std::cout << "ImageCapture: Setting top params...\n" << std::endl;
    top_camera_->setCameraParams();
    camera_info_->set_top_params_ = false;
    top_params_loaded_ = true;
  }

  if (camera_info_->set_bottom_params_) {
    std::cout << "ImageCapture: Setting bottom params..." << std::endl;
    bottom_camera_->setCameraParams();
    camera_info_->set_bottom_params_ = false;
    bottom_params_loaded_ = true;
  }

  if (camera_info_->get_bottom_params_){
    std::cout << "ImageCapture: Getting bottom params..." << std::endl;
    bottom_camera_->getCameraParams();
    camera_info_->get_bottom_params_ = false;
  }

  if (camera_info_->get_top_params_){
    std::cout << "ImageCapture: Getting top params..." << std::endl;
    top_camera_->getCameraParams();
    camera_info_->get_top_params_ = false;
  }

  if(top_params_loaded_ && bottom_params_loaded_ && !camera_info_->cameras_tested_) {
    testCameras();
    camera_info_->cameras_tested_ = true;
  }

  // reset camera params
  if (camera_info_->reset_top_camera_){
    std::cout << "ImageCapture: Reset top camera..." << std::endl;
    top_camera_->reset();
    std::cout << "ImageCapture: Reset top camera complete..." << std::endl;
  }
  if (camera_info_->reset_bottom_camera_){
    std::cout << "ImageCapture: Reset bottom camera..." << std::endl;
    bottom_camera_->reset();
    camera_info_->reset_bottom_camera_ = false;
    std::cout << "ImageCapture: Reset bottom camera complete..." << std::endl;
  }
}

double ImageCapture::getSystemTime() {
  struct timezone tz;
  timeval timeT;
  gettimeofday(&timeT, &tz);
  return timeT.tv_sec + (timeT.tv_usec / 1000000.0);
}
