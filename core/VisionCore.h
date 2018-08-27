#ifndef VISION_CORE_2H7QHCY7
#define VISION_CORE_2H7QHCY7

#include <InterpreterModule.h>
#include <python/PythonModule.h>
#include <common/InterfaceInfo.h> // for core_type
#include <memory/MemoryCache.h>
#include <memory/LogWriter.h>
#include <memory/TextLogger.h>
#include <math/Vector2.h>

class CommunicationModule;
class PerfectLocalizationModule;
class OffFieldLocalizationModule;
class OppModule;
class VisionModule;
class BehaviorModule;
class ButtonModule;
class LEDModule;
class AudioModule;
class ImageCapture;
class ToolPacket;

class LocalizationModule;
class LocalizationMethod {
  public:
    enum Type {
      Default
    };
    static Type DEFAULT;
};

class VisionCore {
public:
  static bool EnableOptimizations();
  VisionCore(CoreType type, bool use_shared_memory, int team_num, int player_num, LocalizationMethod::Type locMethod = LocalizationMethod::DEFAULT);
  ~VisionCore();

  void processVisionFrame();

  void preVision();
  void postVision();

  void logMemory();
  void enableLogging(int frames, double frequency); //Set these both to 0 to bypass frame/freq settings 
  void startDisableLogging();
  void enableTextLogging(const char *filename = NULL);
  void disableTextLogging();

  void updateMemory(MemoryFrame* memory, bool locOnly = false);
  void setMemoryVariables();

#ifndef SWIG
  void setLogSelections(const ToolPacket& selections);
#endif

  MemoryFrame *memory_;
  bool delete_memory_on_destruct_;
  CoreType type_;
  unsigned last_frame_processed_;

  CommunicationModule *communications_;
  InterpreterModule* interpreter_;
  VisionModule *vision_;
  LocalizationModule *localization_;
  OppModule *opponents_;
  BehaviorModule *behavior_;
  ButtonModule *buttons_;
  LEDModule *leds_;
  AudioModule *audio_;

#ifndef SWIG   // Swig can't handle the file IO
  LogWriter* log_;
  std::unique_ptr<TextLogger> textlog_;
#endif
  TextLogger* textlog();

  ImageCapture *image_capture_;

  static VisionCore *inst_;

  FrameInfoBlock *vision_frame_info_;

  BodyModelBlock *vision_body_model_;
  CameraBlock *camera_info_;
  JointBlock *vision_joint_angles_;
  KickRequestBlock *vision_kick_request_;
  OdometryBlock *vision_odometry_;
  SensorBlock *vision_sensors_;
  WalkRequestBlock *vision_walk_request_;
  WalkResponseBlock *vision_walk_response_;
  GameStateBlock *game_state_;
  JointCommandBlock *vision_joint_commands_;
  ProcessedSonarBlock *vision_processed_sonar_;
  KickParamBlock *vision_kick_params_;
  WalkParamBlock *vision_walk_param_;
  ALWalkParamBlock *vision_al_walk_param_;
  WalkInfoBlock *vision_walk_info_;
  WorldObjectBlock *world_objects_;
  RobotVisionBlock *robot_vision_;

  CameraBlock *raw_camera_info_;
  FrameInfoBlock *raw_vision_frame_info_;
  RobotStateBlock *robot_state_;
  RobotInfoBlock *robot_info_;

  void publishData();
  void receiveData();
  void motionLock();
  void motionUnlock();
  static bool isStreaming();

private:
  // synchronized data
  BodyModelBlock *sync_body_model_;
  JointBlock *sync_joint_angles_;
  KickRequestBlock *sync_kick_request_;
  OdometryBlock *sync_odometry_;
  SensorBlock *sync_sensors_;
  WalkRequestBlock *sync_walk_request_;
  WalkResponseBlock *sync_walk_response_;
  JointCommandBlock *sync_joint_commands_;
  ProcessedSonarBlock *sync_processed_sonar_;
  KickParamBlock *sync_kick_params_;
  WalkParamBlock *sync_walk_param_;
  ALWalkParamBlock *sync_al_walk_param_;
  WalkInfoBlock *sync_walk_info_;

  unsigned int frames_to_log_;
  double log_interval_;
  bool log_by_frame_;
  bool disable_log_, is_logging_;
  void enableMemoryLogging();
  void disableMemoryLogging();
  void enableLogging();
  void disableLogging();
  void optionallyWriteLog();

  static const bool useOffFieldLocalization;
private:
  void init(int team_num, int player_num);
  void initMemory();
  void initModules(LocalizationMethod::Type locMethod);
  bool isToolCore();
  Timer vtimer_, camtimer_, logtimer_;
#ifndef SWIG
  std::unique_ptr<ToolPacket> logging_selections_;
#endif
};

#endif /* end of include guard: CORE_2H7QHCY7 */
