#include "LocalizationAnnotation.h"

LocalizationAnnotation::LocalizationAnnotation(Pose2D pose, int frame) {
  pose_ = pose;
  frame_ = frame;
}

void LocalizationAnnotation::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::Key << "pose" << YAML::Value;
  emitter << YAML::BeginMap;
  emitter << YAML::Key << "x" << YAML::Value << pose_.translation.x;
  emitter << YAML::Key << "y" << YAML::Value << pose_.translation.y;
  emitter << YAML::Key << "t" << YAML::Value << pose_.rotation;
  emitter << YAML::EndMap;
  
  emitter << YAML::Key << "frame" << YAML::Value << frame_;
}

void LocalizationAnnotation::deserialize(const YAML::Node& node) {
  const auto& pose = node["pose"];
  pose["x"] >> pose_.translation.x;
  pose["y"] >> pose_.translation.y;
  pose["t"] >> pose_.rotation;
  node["frame"] >> frame_;
}
