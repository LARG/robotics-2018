#ifndef SIMULATION_PATH_H
#define SIMULATION_PATH_H

#include <initializer_list>
#include <list>
#include <common/Random.h>
#include <common/Field.h>
#include <math/Geometry.h>
#include <common/YamlConfig.h>

class SimulationPath : public YamlConfig {
  public:
    SimulationPath() : SimulationPath(Point2D(0,0)) {}
    SimulationPath(Point2D start) { last_ = start; }
    SimulationPath(std::initializer_list<Point2D> l) : SimulationPath() {
      points_ = l;
    }
    int size() const { return points_.size(); }
    bool empty() { return !points_.size(); }
    const Point2D& currentPoint() { return points_.front(); }
    const Point2D& lastPoint() { return last_; }
    std::list<Point2D>::iterator begin() { return points_.begin(); }
    std::list<Point2D>::iterator end() { return points_.end(); }
    void pop();
    void flip();
    static SimulationPath generate(int length = 25, int seed = Random::SEED);
    virtual void deserialize(const YAML::Node& node);
    void serialize(YAML::Emitter& emitter) const;
  private:
    std::list<Point2D> points_;
    Point2D last_;
};

#endif
