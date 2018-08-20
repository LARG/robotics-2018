#pragma once

#include <common/YamlConfig.h>
#include <map>
#include <stdint.h>

class VisionAnnotation;
class LocalizationAnnotation;
class LogViewer;

class AnnotationGroup : public YamlConfig {

  private:
    std::vector<VisionAnnotation*> vannotations_;
    std::vector<LocalizationAnnotation*> lannotations_;

  public:
    AnnotationGroup();
    AnnotationGroup(std::vector<VisionAnnotation*> vannotations);
    AnnotationGroup(const AnnotationGroup& other);
    ~AnnotationGroup();
    std::string path(std::string directory) { return directory + "/annotations.yaml"; }
    void serialize(YAML::Emitter& emitter) const;
    void deserialize(const YAML::Node& node);
    std::vector<VisionAnnotation*> getVisionAnnotations();
    std::vector<LocalizationAnnotation*> getLocalizationAnnotations();
    void save(LogViewer* log);
    void save(std::string directory);
    bool load(LogViewer* log);
    void clear();
    void mergeAnnotations(std::vector<VisionAnnotation*> annotations,int sourceMinFrame, int targetMinFrame, int targetRange);
    void deleteFromFrames(std::vector<int> frames);
    void deleteFromFrame(int frame);
    void addLocalizationAnnotation(LocalizationAnnotation* la);
};
