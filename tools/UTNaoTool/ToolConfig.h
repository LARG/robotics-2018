#ifndef TOOL_CONFIG_H
#define TOOL_CONFIG_H

#include <common/YamlConfig.h>
#include <tool/WorldConfig.h>
#include <tool/RobotControllerConfig.h>
#include <tool/KeyframeConfig.h>

class ToolConfig : public YamlConfig {
  public:
    int logLevelLow;
    int logLevelHigh;
    int moduleTypeIndex;
    int filesLocationIndex;
    bool filesUpdateTimeChecked;
    bool streaming;
    bool onDemand, locOnly, visOnly, locAndVis, coreBehaviors;
    bool logStream;
    int logStart, logEnd, logStep;
    int logFrame;
    std::string logFile;
    WorldConfig worldConfig;
    RobotControllerConfig rcConfig;
    KeyframeConfig kfConfig;
    std::vector<std::string> loggingModules;
    bool enableAudio;

    ToolConfig();
    
    void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
};

#endif
