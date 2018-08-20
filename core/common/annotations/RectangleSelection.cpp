#include <common/annotations/RectangleSelection.h>

void RectangleSelection::init(Point tl, Point br) {
  if(tl.x < br.x) {
    topleft.x = tl.x;
    bottomright.x = br.x;
  }
  else {
    topleft.x = br.x;
    bottomright.x = tl.x;
  }
  if(tl.y < br.y){
    topleft.y = tl.y;
    bottomright.y = br.y;
  }
  else {
    topleft.y = br.y;
    bottomright.y = tl.y;
  }
}

void RectangleSelection::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::Key << "type" << YAML::Value << SelectionTypeMethods::getName(getSelectionType());
  emitter << YAML::Key << "top" << YAML::Value << topleft.y;
  emitter << YAML::Key << "left" << YAML::Value << topleft.x;
  emitter << YAML::Key << "bottom" << YAML::Value << bottomright.y;
  emitter << YAML::Key << "right" << YAML::Value << bottomright.x;
}

void RectangleSelection::deserialize(const YAML::Node& node) {
  int top, left, right, bottom;
  node["top"] >> top;
  node["left"] >> left;
  node["right"] >> right;
  node["bottom"] >> bottom;
  topleft = Point(left, top);
  bottomright = Point(right,bottom);
}

std::vector<Point> RectangleSelection::getEnclosedPoints() const {
  std::vector<Point> points;
  for(int i = topleft.x; i <= bottomright.x; i++) {
    for(int j = topleft.y; j <= bottomright.y; j++) {
      Point p(i,j);
      points.push_back(p);
    }
  }
  return points;
}

bool RectangleSelection::enclosesPoint(int x, int y) const {
  if(x >= topleft.x && y >= topleft.y && x < bottomright.x && y < bottomright.y)
    return true;
  return false;
}

Selection* RectangleSelection::copy() const {
  RectangleSelection* r = new RectangleSelection();
  r->topleft = this->topleft;
  r->bottomright = this->bottomright;
  return r;
}

Point RectangleSelection::getCenter() const {
  return Point( (this->bottomright.x + this->topleft.x) / 2, (this->bottomright.y + this->topleft.y) / 2 );
}
