#include <common/RobotCalibration.h>

RobotCalibration::RobotCalibration() {
  enabled = true;
  topFOVx = bottomFOVx = 0;
  topFOVy = bottomFOVy = 0;
  memset(jointValues_, 0, NUM_JOINTS * sizeof(float));
  memset(sensorValues_, 0, NUM_SENSORS * sizeof(float));
  memset(dimensionValues_, 0, RobotDimensions::NUM_DIMENSIONS * sizeof(float));
  k1 = -0.0958081087594769;
  k2 =  0.1014458308701546;
  p1 =  0.01421471087609939;
  p2 = -0.004138935948982259;
  k3 = -0.04019608341725905;
  poseX = poseY = poseZ = poseTheta = 0;
}


void RobotCalibration::applyJoints(float* joints) {
  for(int i = 0; i < NUM_JOINTS; i++)
    joints[i] += jointValues_[i];
}

void RobotCalibration::applySensors(float* sensors) {
  for(int i = 0; i < NUM_SENSORS; i++)
    sensors[i] += sensorValues_[i];
}

void RobotCalibration::applyDimensions(float* dimensions) {
  for(int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++)
    dimensions[i] += dimensionValues_[i];
}

void RobotCalibration::revertJoints(float* joints) {
  for(int i = 0; i < NUM_JOINTS; i++)
    joints[i] -= jointValues_[i];
}

void RobotCalibration::revertSensors(float* sensors) {
  for(int i = 0; i < NUM_SENSORS; i++)
    sensors[i] -= sensorValues_[i];
}

void RobotCalibration::revertDimensions(float* dimensions) {
  for(int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++)
    dimensions[i] -= dimensionValues_[i];
}

void RobotCalibration::deserialize(const YAML::Node& node) {
  YAML_DESERIALIZE(node, enabled);
  YAML_DESERIALIZE(node, poseX);
  YAML_DESERIALIZE(node, poseY);
  YAML_DESERIALIZE(node, poseZ);
  YAML_DESERIALIZE(node, poseTheta);
  YAML_DESERIALIZE(node, topFOVx);
  YAML_DESERIALIZE(node, topFOVy);
  YAML_DESERIALIZE(node, bottomFOVx);
  YAML_DESERIALIZE(node, bottomFOVy);
  YAML_DESERIALIZE(node, k1);
  YAML_DESERIALIZE(node, k2);
  YAML_DESERIALIZE(node, k3);
  YAML_DESERIALIZE(node, p1);
  YAML_DESERIALIZE(node, p2); 
  for(int i = 0; i < NUM_JOINTS; i++)
    node[JointNames[i]] >> jointValues_[i];

  for(int i = 0; i < NUM_SENSORS; i++)
    node[SensorNames[i]] >> sensorValues_[i];

  for(int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++)
    node[DimensionNames[i]] >> dimensionValues_[i];
}

void RobotCalibration::serialize(YAML::Emitter& emitter) const {
  YAML_SERIALIZE(emitter, enabled);
  YAML_SERIALIZE(emitter, poseX);
  YAML_SERIALIZE(emitter, poseY);
  YAML_SERIALIZE(emitter, poseZ);
  YAML_SERIALIZE(emitter, poseTheta);
  YAML_SERIALIZE(emitter, topFOVx);
  YAML_SERIALIZE(emitter, topFOVy);
  YAML_SERIALIZE(emitter, bottomFOVx);
  YAML_SERIALIZE(emitter, bottomFOVy);
  YAML_SERIALIZE(emitter, k1);
  YAML_SERIALIZE(emitter, k2);
  YAML_SERIALIZE(emitter, k3);
  YAML_SERIALIZE(emitter, p1);
  YAML_SERIALIZE(emitter, p2); 
  for(int i = 0; i < NUM_JOINTS; i++)
    emitter << YAML::Key << JointNames[i] << YAML::Value << jointValues_[i];

  for(int i = 0; i < NUM_SENSORS; i++)
    emitter << YAML::Key << SensorNames[i] << YAML::Value << sensorValues_[i];

  for(int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++)
    emitter << YAML::Key << DimensionNames[i] << YAML::Value << dimensionValues_[i];
}
