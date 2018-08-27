#pragma once

#include <common/YamlConfig.h>
#include <common/annotations/SelectionType.h>
#include <common/Camera.h>
#include <vision/enums/Colors.h>

class AnnotationConfig : public YamlConfig {
  public:
    AnnotationConfig();
    Color color;
    SelectionType selection_type;
    Camera::Type camera;
    bool auto_create;
    
  private:
    void deserialize(const YAML::Node& node) override;
    void serialize(YAML::Emitter& emitter) const override;
};
