#include "JointDataset.h"

using namespace Eigen;

void JointMeasurement::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, left);
  YAML_DESERIALIZE(node, joints);
  const auto& cnode = node["corners"];
  for(YAML::Iterator it = cnode.begin(); it != cnode.end(); ++it) {
    float x, y;
    *it >> x;
    ++it;
    *it >> y;
    corners.push_back(Vector2f(x,y));
  }
}

void JointMeasurement::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, left);
  YAML_SERIALIZE(emitter, joints);
  emitter << YAML::Key << "corners";
  emitter << YAML::Value;
  emitter << YAML::BeginSeq;
  for(auto corner : corners)
    emitter << corner[0] << corner[1];
  emitter << YAML::EndSeq;
}

void JointDataset::deserialize(const YAML::Node& node) {
  for(auto it = node.begin(); it != node.end(); ++it) {
    JointMeasurement m;
    m.deserialize(*it);
    push_back(m);
  }
}

void JointDataset::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::BeginSeq;
  for(const auto& m : *this)
    m.serialize(emitter);
  emitter << YAML::EndSeq;
}


