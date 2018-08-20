#pragma once

#include <map>
#include <math/Pose2D.h>
#include <common/annotations/Annotation.h>

class LocalizationAnnotation : public Annotation {
  public:
    LocalizationAnnotation() = default;
    LocalizationAnnotation(Pose2D pose, int frame);
    const Pose2D& pose() const { return pose_; }
    int frame() const { return frame_; }
  private:
    Pose2D pose_;
    int frame_;
  protected:
    void serialize(YAML::Emitter& emitter) const override;
    void deserialize(const YAML::Node& node) override;
};
