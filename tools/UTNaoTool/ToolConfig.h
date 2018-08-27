#ifndef TOOL_CONFIG_H
#define TOOL_CONFIG_H

#include <common/YamlConfig.h>
#include <tool/LogWindowConfig.h>
#include <tool/WorldConfig.h>
#include <tool/VisionConfig.h>
#include <tool/AnnotationConfig.h>
#include <tool/MemorySelectConfig.h>
#include <tool/RobotControllerConfig.h>
#include <tool/KeyframeConfig.h>

class ToolConfig : public YamlConfig {
  public:
    ToolConfig();
    ~ToolConfig();
    int filesLocationIndex;
    bool filesUpdateTimeChecked;
    bool streaming;
    bool onDemand, bypassVision, visionOnly, fullProcess, coreBehaviors;
    bool logStream;
    int logStart, logEnd, logStep;
    int logFrame;
    std::string logPath;
    LogWindowConfig logWindowConfig;
    WorldConfig worldConfig;
    RobotControllerConfig rcConfig;
    KeyframeConfig kfConfig;
    VisionConfig visionConfig;
    AnnotationConfig annotationConfig;
    MemorySelectConfig memorySelectConfig;
    std::vector<std::string> loggingModules;
    bool enableAudio;

  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};

#endif
