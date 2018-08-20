#pragma once

#include <common/annotations/Selection.h>
#include <vector>
#include <math/Polygon.h>

class PolygonSelection : public Selection {
  protected:
    mutable std::unique_ptr<Polygon> polygon_;
    std::vector<Point> vertices_;
    
    void getPolygon() const {
      int count = vertices_.size();
      std::vector<Point> points;
      for(int i=0; i < count; i++)
        points.push_back(Point(vertices_[i].x,vertices_[i].y));
      polygon_ = std::make_unique<Polygon>(points);
    }
  public:
    PolygonSelection() = default;
    PolygonSelection(const PolygonSelection& p) {
      *this = p;
    }
    PolygonSelection(PolygonSelection&& p) {
      *this = p;
    }
    PolygonSelection(std::vector<Point> v) {
      vertices_ = std::vector<Point>(v);
    }
    PolygonSelection& operator=(const PolygonSelection& p) {
      Selection::operator=(p);
      polygon_ = std::make_unique<Polygon>(*p.polygon_);
      vertices_ = p.vertices_;
      return *this;
    }
    PolygonSelection& operator=(PolygonSelection&& p) {
      Selection::operator=(p);
      polygon_ = std::move(p.polygon_);
      vertices_ = std::move(p.vertices_);
      return *this;
    }

    std::vector<Point> getVertices() const {
      uint16_t count = vertices_.size();
      std::vector<Point> offsetVertices;
      for(uint16_t i = 0; i < count; i++) {
        offsetVertices.push_back(vertices_[i] + getOffset());
      }
      return offsetVertices;
    }
    void addVertex(Point p) {
      vertices_.push_back(p);
    }
    void serialize(YAML::Emitter& emitter) const final {
      emitter << YAML::Key << "type" << YAML::Value << SelectionTypeMethods::getName(getSelectionType());
      emitter << YAML::Key << "vertices" << YAML::Value;

      emitter << YAML::BeginSeq;
      for(uint16_t i = 0; i < vertices_.size(); i++) {
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "x" << YAML::Value << vertices_[i].x;
        emitter << YAML::Key << "y" << YAML::Value << vertices_[i].y;
        emitter << YAML::EndMap;
      }
      emitter << YAML::EndSeq;
    }
    void deserialize(const YAML::Node& node) final {
      const YAML::Node& v = node["vertices"];
      for(uint16_t i = 0; i < v.size(); i++){
        int x, y;
        const YAML::Node& vertex = v[i];
        vertex["x"] >> x;
        vertex["y"] >> y;
        Point point(x,y);
        vertices_.push_back(point);
      }
    }
    virtual SelectionType getSelectionType() const override {
      return SelectionType::Polygon;
    }
    std::vector<Point> getEnclosedPoints() const final {
      if(!polygon_) getPolygon();
      return polygon_->enclosedPoints();
    }
    bool enclosesPoint(int x, int y) const final {
      if(!polygon_) getPolygon();
      return polygon_->enclosesPoint(x,y);
    }
    virtual Selection* copy() const override {
      PolygonSelection* p = new PolygonSelection();
      int count = vertices_.size();
      for(int i = 0; i < count; i++)
        p->vertices_.push_back(vertices_[i]);
      return p;
    }
    Point getCenter() const final {
      Point average(0,0);
      for(uint16_t i = 0; i < vertices_.size(); i++) {
        average.x += vertices_[i].x;
        average.y += vertices_[i].y;
      }
      average.x /= vertices_.size();
      average.y /= vertices_.size();
      return average;
    }
};
