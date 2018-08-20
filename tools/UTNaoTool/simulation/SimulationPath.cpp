#include "SimulationPath.h"

void SimulationPath::flip() {
  for(auto& p : points_)
    p = -p;
  last_ = -last_;
}

void SimulationPath::pop() {
  last_ = points_.front();
  points_.pop_front();
}

SimulationPath SimulationPath::generate(int length, int seed) {
  auto rand = Random(seed);
  SimulationPath path;
  for(int i = 0; i < length; i++) {
    int x = rand.sampleU(-FIELD_X / 2, FIELD_X / 2);
    int y = rand.sampleU(-FIELD_Y / 2, FIELD_Y / 2);
    path.points_.push_back(Point2D(x,y));
  }
  return path;
}

void SimulationPath::deserialize(const YAML::Node& node) {
  const YAML::Node& points = node["points"];
  for(YAML::Iterator it=points.begin();it!=points.end();++it) {
    float x,y;
    const YAML::Node& pnode = *it;
    pnode["x"] >> x;
    pnode["y"] >> y;
    auto p = Point2D(x,y);
    points_.push_back(p);
  }
}

void SimulationPath::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::Key << "points" << YAML::Value;
  emitter << YAML::BeginSeq;
  for(auto& p : points_) {
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "x" << YAML::Value << p.x;
    emitter << YAML::Key << "y" << YAML::Value << p.y;
    emitter << YAML::EndMap;
  }
  emitter << YAML::EndSeq;
}
