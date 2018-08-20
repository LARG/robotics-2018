#ifndef VISION_99KDYIX5
#define VISION_99KDYIX5

#include <Module.h>
#include <common/RobotInfo.h>
#include <vision/ImageProcessor.h>
#include <vision/VisionBlocks.h>

class FrameInfoBlock;
class JointBlock;
class RobotVisionBlock;
class SensorBlock;
class ImageBlock;
class WorldObjectBlock;
class RobotStateBlock;
class BodyModelBlock;
class CameraBlock;
class RobotInfoBlock;
class GameStateBlock;

/// @ingroup vision
class VisionModule: public Module {

public:
  VisionModule();
  ~VisionModule();

  void specifyMemoryBlocks();
  void specifyMemoryDependency();
  void initSpecificModule();
  void processFrame();
  void updateTransforms();

  bool loadColorTables();
  bool loadColorTable(Camera::Type camera, std::string fileName, bool fullpath=false);

  ImageParams *top_params_, *bottom_params_;
  ImageProcessor *top_processor_, *bottom_processor_;

  unsigned char* bottomColorTable;
  unsigned char* topColorTable;
  string bottomColorTableName;
  string topColorTableName;

private:
  FrameInfoBlock *vision_frame_info_;
  JointBlock *joint_angles_;
  RobotVisionBlock *robot_vision_;
  SensorBlock *sensors_;
  ImageBlock *image_;
  WorldObjectBlock *world_objects_;
  RobotStateBlock *robot_state_;
  BodyModelBlock *body_model_;
  CameraBlock *camera_info_;
  RobotInfoBlock *robot_info_;
  GameStateBlock *game_state_;

  VisionBlocks* vblocks_;

  bool isBottomCamera();
  bool useSimColorTable();
  std::string getDataBase();
  int getRobotId();
  int getTeamColor();
  bool areFeetOnGround();
};

#endif /* end of include guard: VISION_99KDYIX5 */
