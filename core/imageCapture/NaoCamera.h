/**
* \file Platform/linux/NaoCamera.h
* Interface to the Nao camera using linux-uvc.
* \author Colin Graf
*/

#ifndef _NaoCamera_H_
#define _NaoCamera_H_

#include "V4L2Constants.h"
#include <common/RobotInfo.h>
#include <common/CameraParams.h>
#include <common/ImageParams.h>
#include <vision/VisionConstants.h>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>

// struct buffer {
//   void * start;
//   size_t length;
// };

/**
* \class NaoCamera
* Interface class to the Nao camera.
*/
class NaoCamera
{
public:
  void updateBuffers();
  void enqueueBuffer();
  void dequeueBuffer();
  void swapBuffers();
  uint8_t* getImage();
  unsigned getTimeStamp() const;
  int getControlSetting(unsigned int id);
  bool setControlSetting(unsigned int id, int value);
  void setCameraParams();
  void getCameraParams();
  void reset();
  bool selfTest();
  ~NaoCamera();

  void enableAutoWB();
  int lockWB();

protected:
    bool vflip_, hflip_;
    std::string device_path_;
    CameraParams& camera_params_;
    CameraParams& read_camera_params_;
    NaoCamera(const ImageParams& iparams, CameraParams&, CameraParams&);
    void init();
    int exposure; 

private:
  const ImageParams& iparams_;
  int videoDeviceFd;
  bool initialized;
  enum
  {
    frameBufferCount = 3, /**< Amount of available frame buffers. */
  };
  void* mem[frameBufferCount]; /**< Frame buffer addresses. */
  int memLength[frameBufferCount]; /**< The length of each frame buffer. */
  struct v4l2_buffer* buf; /**< Reusable parameter struct for some ioctl calls. */
  struct v4l2_buffer *nextBuf, *prevBuf; /**< The last dequeued frame buffer. */
  unsigned timeStamp, /**< Timestamp of the last captured image. */
           storedTimeStamp; /**< Timestamp when the next image recording starts. */

  void setDefaultSettings();
  void initOpenVideoDevice();
  void initSetImageFormat();
  void initSetFrameRate();
  void initRequestAndMapBuffers();
  void initQueueAllBuffers();
  void unmapAndFreeBuffers();
  void enableStreaming();
  void disableStreaming();

  void displayAvailableSettings();
  void enumerateSettingMenu();

  v4l2_queryctrl queryctrl;
  v4l2_querymenu querymenu;
};

#endif // _NaoCamera_H_
