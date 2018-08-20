#ifndef CAMERABLOCK_D16C3JJ3
#define CAMERABLOCK_D16C3JJ3

#include <memory/MemoryBlock.h>
#include <common/CameraParams.h>
#include <schema/gen/CameraBlock_generated.h>

DECLARE_INTERNAL_SCHEMA(struct CameraBlock : public MemoryBlock {
  public:
    SCHEMA_METHODS(CameraBlock);
    CameraBlock():
      get_bottom_params_(false),
      get_top_params_(false),
      set_bottom_params_(false),
      set_top_params_(false),
      reset_top_camera_(false),
      reset_bottom_camera_(false),
      cameras_tested_(false),
      comm_module_request_received_(false),
      calibrate_white_balance_(false)
        {
          header.version = 7;
          header.size = sizeof(CameraBlock);
        }

    void copyFromImageCapture(CameraBlock *raw) {

      //(piyushk) nastiest hack possible
      // Request was received between last publishData call this receiveData call 
      if (comm_module_request_received_) {
        comm_module_request_received_ = false;
      } else {
        get_bottom_params_ = false;
        get_top_params_ = false;
        set_bottom_params_ = false;
        set_top_params_ = false;
        reset_top_camera_ = false;
        reset_bottom_camera_ = false;
      }
      cameras_tested_ = raw->cameras_tested_;
      // copy params read from camera, no need to copy ones we're sending
      read_params_bottom_camera_ = raw->read_params_bottom_camera_;
      read_params_top_camera_ = raw->read_params_top_camera_;
    }

    void copyToImageCapture(CameraBlock *raw) {
      comm_module_request_received_ = false; // Does not matter
      raw->get_bottom_params_ = raw->get_bottom_params_ || get_bottom_params_;
      raw->get_top_params_ = raw->get_top_params_ || get_top_params_;
      raw->set_bottom_params_ = raw->set_bottom_params_ || set_bottom_params_;
      raw->set_top_params_ = raw->set_top_params_ || set_top_params_;
      raw->reset_top_camera_ = raw->reset_top_camera_ || reset_top_camera_;
      raw->reset_bottom_camera_ = raw->reset_bottom_camera_ || reset_bottom_camera_;
      // copy params we're sending to cam, no need to copy ones we want to read
      raw->params_bottom_camera_ = params_bottom_camera_;
      raw->params_top_camera_ = params_top_camera_;
    }

    SCHEMA_FIELD(CameraParams params_bottom_camera_);
    SCHEMA_FIELD(CameraParams params_top_camera_);

    SCHEMA_FIELD(CameraParams read_params_bottom_camera_);
    SCHEMA_FIELD(CameraParams read_params_top_camera_);
    SCHEMA_FIELD(bool get_bottom_params_);
    SCHEMA_FIELD(bool get_top_params_);
    SCHEMA_FIELD(bool set_bottom_params_);
    SCHEMA_FIELD(bool set_top_params_);
    SCHEMA_FIELD(bool reset_top_camera_);
    SCHEMA_FIELD(bool reset_bottom_camera_);
    SCHEMA_FIELD(bool cameras_tested_);
    SCHEMA_FIELD(bool comm_module_request_received_);
    SCHEMA_FIELD(bool calibrate_white_balance_);
});

#endif /* end of include guard: CAMERABLOCK_D16C3JJ3 */
