#ifndef INTERPRETER_MODULE_H
#define INTERPRETER_MODULE_H

#include <Module.h>
#include <math/Pose2D.h>
#include <math/Pose3D.h>
#include <memory/Blocks.h>
#include <motion/KickParameters.h>
#include <common/Profiling.h>
#include <common/RobotConfig.h>
#include <communications/CommInfo.h>

class VisionCore;

class InterpreterModule: public Module {
  public:
    BehaviorBlock *behavior_;
    CameraBlock *camera_block_;
    FrameInfoBlock *vision_frame_info_;
    GameStateBlock *game_state_;
    JointBlock *joint_angles_;
    KickRequestBlock *kick_request_;
    OdometryBlock *odometry_;
    RobotStateBlock *robot_state_;
    SensorBlock *sensors_;
    KickParamBlock *kick_params_;
    WalkParamBlock *walk_param_;
    WalkRequestBlock *walk_request_;
    WalkResponseBlock *walk_response_;
    WorldObjectBlock *world_objects_;
    TeamPacketsBlock *team_packets_;
    OpponentBlock *opponents_;
    BehaviorParamBlock *behavior_params_;
    JointCommandBlock *joint_commands_;
    ProcessedSonarBlock *vision_processed_sonar_;
    ALWalkParamBlock *al_walk_param_;
    WalkInfoBlock *walk_info_;
    RobotVisionBlock *robot_vision_;
    BodyModelBlock *body_model_;
    RobotInfoBlock *robot_info_;
    SpeechBlock *speech_;
    LocalizationBlock *localization_;
    ImageBlock *image_;
    AudioProcessingBlock *audio_processing_;

    std::vector<float> joint_values_, joint_stiffness_;
    std::vector<float> sensor_values_;

    InterpreterModule(VisionCore* core);
    ~InterpreterModule();

    void specifyMemoryDependency();
    void specifyMemoryBlocks();
    virtual void initSpecificModule() = 0;
    virtual void processFrame() = 0;
    virtual void processBehaviorFrame() = 0;
    virtual void doStrategyCalculations() = 0;
    virtual void updateModuleMemory(MemoryFrame *memory) = 0;
    virtual void saveKickParameters(const KickParameters& kp) = 0;
    virtual void start() = 0;
    virtual void restart() = 0;
    virtual void initFromMemory() { }
    virtual void runBehavior(std::string behavior) { }
    void updatePercepts();
    void readConfig();

    Timer timer_;
    bool is_ok_;
    bool restart_requested_;
    // helpers
    bool getBool(bool *arr, int ind);
    void setBool(bool *arr, int ind, bool val);
    float getFloat(float *arr,int ind);
    void setFloat(float *arr, int ind, float val);
    double getDouble(double *arr,int ind);
    void setDouble(double *arr, int ind, double val);
    int getInt(int *arr,int ind);
    void setInt(int *arr, int ind, int val);
    unsigned char getUchar(unsigned char *arr,int ind);
    void setUchar(unsigned char *arr, int ind, unsigned char val);
    std::string getString(std::string *arr, int ind);
    Pose2D getPose2D(Pose2D *arr,int ind);
    void setPose2D(Pose2D *arr,int ind, Pose2D val);
    Pose3D getPose3D(Pose3D *arr,int ind);
    void setPose3D(Pose3D *arr,int ind, Pose3D val);
    Pose3D* getPose3DPtr(Pose3D *arr,int ind);
    void log(int level, std::string message);
  protected:
    VisionCore* core_;
};

#endif /* end of include guard: VISION_99KDYIX5 */
