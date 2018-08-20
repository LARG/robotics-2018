#include <common/Keyframe.h>

using namespace std;

Keyframe::Keyframe(string name) : name(name) {
  for(int i = 0; i < NUM_JOINTS; i++)
    joints[i] = 0.0f;
  frames = 100;
}

Keyframe::Keyframe(std::array<float, NUM_JOINTS> joints, int frames) : joints(joints), frames(frames) {
}

void Keyframe::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, name);
  YAML_DESERIALIZE(node, frames);
  const auto& jnode = node["joints"];
  for(int i = 0; i < NUM_JOINTS; i++) {
    jnode[JointNames[i]] >> joints[i];
    joints[i] *= DEG_T_RAD;
  }
}

void Keyframe::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, name);
  YAML_SERIALIZE(emitter, frames);
  emitter << YAML::Key << "joints" << YAML::Value;
  emitter << YAML::BeginMap;
  for(int i = 0; i < NUM_JOINTS; i++)
    emitter << YAML::Key << JointNames[i] << YAML::Value << joints[i] * RAD_T_DEG;
  emitter << YAML::EndMap;
}

void KeyframeSequence::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, keyframes);
}

void KeyframeSequence::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, keyframes);
}

