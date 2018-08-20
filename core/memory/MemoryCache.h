#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#include <string>
#include <unordered_set>
   
class MemoryFrame;
class WorldObjectBlock;
class LocalizationBlock;
class TeamPacketsBlock;
class FrameInfoBlock;
class RobotStateBlock;
class GameStateBlock;
class OdometryBlock;
class JointBlock;
class BehaviorBlock;
class ProcessedSonarBlock;
class DelayedLocalizationBlock;
class BodyModelBlock;
class ImageBlock;
class CameraBlock;
class RobotVisionBlock;
class RobotInfoBlock;
class SensorBlock;
class OpponentBlock;
class BehaviorParamBlock;
class WalkRequestBlock;
class WalkResponseBlock;
class KickRequestBlock;
class WalkParamBlock;
class JointCommandBlock;
class WalkInfoBlock;
class AudioProcessingBlock;
class SimTruthDataBlock;
class CalibrationBlock;

class MemoryCache {
  public:
    WorldObjectBlock* world_object;
    LocalizationBlock* localization_mem;
    TeamPacketsBlock* team_packets;
    FrameInfoBlock* frame_info;
    RobotStateBlock* robot_state;
    GameStateBlock* game_state;
    OdometryBlock* odometry;
    JointBlock* joint;
    BehaviorBlock* behavior;
    ProcessedSonarBlock* sonar;
    DelayedLocalizationBlock* delayed_localization;
    BodyModelBlock* body_model;
    ImageBlock* image;
    CameraBlock* camera;
    RobotVisionBlock* robot_vision;
    RobotInfoBlock* robot_info;
    SensorBlock* sensor;
    OpponentBlock* opponent_mem;
    BehaviorParamBlock* behavior_params;
    WalkRequestBlock* walk_request;
    WalkResponseBlock* walk_response;
    KickRequestBlock* kick_request;
    WalkParamBlock* walk_params;
    JointCommandBlock* joint_command;
    WalkInfoBlock* walk_info;
    AudioProcessingBlock* audio_processing;
    SimTruthDataBlock* sim_truth;
    CalibrationBlock* calibration;

    MemoryFrame* memory;

    MemoryCache();
    MemoryCache(MemoryFrame* memory);
    MemoryCache(MemoryFrame& memory);
    void clear();
    void fill(MemoryFrame* memory);
    void fill(MemoryFrame& memory);
    void fill(MemoryFrame* memory, const std::unordered_set<std::string>& blocks);
    void fill(MemoryFrame& memory, const std::unordered_set<std::string>& blocks);

    static MemoryCache read(MemoryFrame* memory);
    static MemoryCache read(MemoryFrame& memory);
    static MemoryCache create(int team, int player);
};

#endif
