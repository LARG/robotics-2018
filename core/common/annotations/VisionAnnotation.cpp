#include <common/annotations/VisionAnnotation.h>

VisionAnnotation::VisionAnnotation() {
}

VisionAnnotation::VisionAnnotation(std::string name) {
  name_ = name;
}

void VisionAnnotation::addSelection(Selection* selection) {
  selections_.push_back(selection);
}

void VisionAnnotation::removeSelection(Selection* selection) {
  selections_.erase(std::remove(selections_.begin(), selections_.end(), selection), selections_.end());
}

bool VisionAnnotation::hasSelection(Selection* selection) {
  return std::find(selections_.begin(), selections_.end(), selection) != selections_.end();
}

std::string VisionAnnotation::getName() {
  return name_;
}

void VisionAnnotation::setName(std::string name) {
  name_ = name;
}

const std::vector<Selection*> VisionAnnotation::getSelections() const {
  return selections_;
}

bool VisionAnnotation::isInFrame(int frame) {
  return frame >= minFrame_ && frame <= maxFrame_;
}
void VisionAnnotation::setMinFrame(int frame) {
  minFrame_ = frame;
}
void VisionAnnotation::setMaxFrame(int frame) {
  maxFrame_ = frame;
}

int VisionAnnotation::getMinFrame() {
  return minFrame_;
}

int VisionAnnotation::getMaxFrame() {
  return maxFrame_;
}

void VisionAnnotation::setColor(Color c) {
  color_ = c;
}

Color VisionAnnotation::getColor() {
  return color_;
}

Camera::Type VisionAnnotation::getCamera() {
  return camera_;
}

void VisionAnnotation::setCamera(Camera::Type camera) {
  camera_ = camera;
}

std::vector<Point> VisionAnnotation::getEnclosedPoints() {
  uint16_t count = selections_.size();
  std::vector<Point> points;
  for(uint16_t i = 0; i < count; i++) {
    Selection* s = selections_[i];
    std::vector<Point> spoints = s->getEnclosedPoints();
    uint16_t scount = spoints.size();
    for(uint16_t j=0; j< scount; j++) {
      points.push_back(spoints[j]);
    }
  }
  return points;
}

std::vector<Point> VisionAnnotation::getEnclosedPoints(int frame) {
  if(centerPoints_.empty()) return getEnclosedPoints();
  uint16_t count = selections_.size();
  std::vector<Point> points;
  for(uint16_t i = 0; i < count; i++) {
    Selection* s = selections_[i];
    std::vector<Point> spoints = s->getEnclosedPoints(frame);
    uint16_t scount = spoints.size();
    for(uint16_t j=0; j< scount; j++) {
      points.push_back(spoints[j]);
    }
  }
  return points;
}

bool VisionAnnotation::enclosesPoint(int x, int y) {
  uint16_t count = selections_.size();
  for(uint16_t i = 0; i < count; i++)
    if(selections_[i]->enclosesPoint(x,y))
      return true;
  return false;
}

bool VisionAnnotation::enclosesPoint(int x, int y, int frame) {
  if(centerPoints_.empty()) return enclosesPoint(x,y);
  uint16_t count = selections_.size();
  for(uint16_t i = 0; i < count; i++)
    if(selections_[i]->enclosesPoint(x,y,frame));
      return true;
  return false;
}

void VisionAnnotation::serialize(YAML::Emitter& emitter) const {
  emitter << YAML::Key << "name" << YAML::Value << name_;
  emitter << YAML::Key << "minFrame" << YAML::Value << minFrame_;
  emitter << YAML::Key << "maxFrame" << YAML::Value << maxFrame_;
  emitter << YAML::Key << "color" << YAML::Value << color_;
  emitter << YAML::Key << "camera" << YAML::Value << camera_;
  emitter << YAML::Key << "isSample" << YAML::Value << isSample_;
  emitter << YAML::Key << "centerPoints" << YAML::Value;
  emitter << YAML::BeginSeq;
  for(std::map<int,Point>::const_iterator it = centerPoints_.begin(); it != centerPoints_.end(); it++) {
    int frame = (*it).first;
    Point newCenter = (*it).second;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "frame" << YAML::Value << frame;
    emitter << YAML::Key << "x" << YAML::Value << newCenter.x;
    emitter << YAML::Key << "y" << YAML::Value << newCenter.y;
    emitter << YAML::EndMap;
  }
  emitter << YAML::EndSeq;
  emitter << YAML::Key << "selections" << YAML::Value;
  emitter << YAML::BeginSeq;
  for(uint16_t i = 0; i < selections_.size(); i++) {
    emitter << *selections_[i];
  }
  emitter << YAML::EndSeq;
}

void VisionAnnotation::deserialize(const YAML::Node& node) {
  node["name"] >> name_;
  node["minFrame"] >> minFrame_;
  node["maxFrame"] >> maxFrame_;
  node["color"] >> color_;
  node["camera"] >> camera_;
  if(const YAML::Node *pSample = node.FindValue("isSample"))
    *pSample >> isSample_;
  if(const YAML::Node *pCenterPoints = node.FindValue("centerPoints")) {
    const YAML::Node& centerPoints = *pCenterPoints;
    for(uint16_t i = 0; i < centerPoints.size(); i++) {
      const YAML::Node& centerPoint = centerPoints[i];
      int x, y, frame;
      centerPoint["x"] >> x;
      centerPoint["y"] >> y;
      centerPoint["frame"] >> frame;
      setCenterPoint(x,y,frame);
    }
  }
  const YAML::Node& selections = node["selections"];
  for(uint16_t i = 0; i < selections.size(); i++) {
    const YAML::Node& selection = selections[i];
    std::string type;
    selection["type"] >> type;
    if(type == "Manual") {
      ManualSelection* manual = new ManualSelection();
      selection >> *manual;
      if(manual->getVertices().size() == 0) continue;
      selections_.push_back(manual);
    }
    if(type == "Polygon") {
      PolygonSelection* polygon = new PolygonSelection();
      selection >> *polygon;
      if(polygon->getVertices().size() == 0) continue;
      selections_.push_back(polygon);
    }
    else if (type == "Ellipse") {
      EllipseSelection* ellipse = new EllipseSelection();
      selection >> *ellipse;
      selections_.push_back(ellipse);
    }
    else if (type == "Rectangle") {
      RectangleSelection* rectangle = new RectangleSelection();
      selection >> *rectangle;
      selections_.push_back(rectangle);
    }
    else if (type == "Square") {
      SquareSelection* square = new SquareSelection();
      selection >> *square;
      selections_.push_back(square);
    }
  }
  updateSelectionOffsets();
}

VisionAnnotation* VisionAnnotation::copy() {
  VisionAnnotation* annotation = new VisionAnnotation();
  annotation->name_ = this->name_;
  annotation->minFrame_ = this->minFrame_;
  annotation->maxFrame_ = this->maxFrame_;
  annotation->color_ = this->color_;
  annotation->camera_ = this->camera_;
  annotation->centerPoints_ = this->centerPoints_;
  annotation->isSample_ = this->isSample_;
  for(uint16_t i = 0; i < selections_.size(); i++)
    annotation->selections_.push_back(selections_[i]->copy());
  return annotation;
}

bool VisionAnnotation::isSample() {
  return isSample_;
}

void VisionAnnotation::setSample(bool isSample) {
  isSample_ = isSample;
}

void VisionAnnotation::setCenterPoint(int x, int y, int frame) {
  centerPoints_[frame] = Point(x,y);
  updateSelectionOffsets();
}

Point VisionAnnotation::getCenter() {
  Point average(0,0);
  if(selections_.size() == 0)
    return average;
  for(uint16_t i = 0; i < selections_.size(); i++) {
    Selection* selection = selections_[i];
    Point center = selection->getCenter();
    average.x += center.x;
    average.y += center.y;
  }
  average.x /= selections_.size();
  average.y /= selections_.size();
  return average;
}

Point VisionAnnotation::getCenter(int frame) {
  auto it = centerPoints_.find(frame);
  if(it == centerPoints_.end())
    return getCenter();
  return it->second;
}

void VisionAnnotation::updateSelectionOffsets() {
  Point origCenter = getCenter();
  for(uint16_t i = 0; i < selections_.size(); i++) {
    Selection* selection = selections_[i];
    for(auto kvp : centerPoints_) {
      int frame = kvp.first;
      auto newCenter = kvp.second;
      Point offset = newCenter - origCenter;
      selection->setOffset(frame, offset);
    }
  }
}

void VisionAnnotation::setCurrentFrame(int frame) {
  for(uint16_t i = 0; i < selections_.size(); i++)
    selections_[i]->setCurrentFrame(frame);
}

void VisionAnnotation::clearCenterPoints() {
  centerPoints_.clear();
  for(uint16_t i = 0; i < selections_.size(); i++) {
    Selection* selection = selections_[i];
    selection->clearOffsets();
  }
}

void VisionAnnotation::remapCenterPoints(int offset) {
  std::map<int,Point> original = centerPoints_;
  centerPoints_.clear();
  for(std::map<int,Point>::iterator it = original.begin(); it != original.end(); it++) {
    int frame = (*it).first;
    Point point = (*it).second;
    centerPoints_[frame + offset] = point;
  }
}
        
bool VisionAnnotation::operator==(const VisionAnnotation& other) {
  // Compare everything except for frames
  if(name_ != other.name_) return false;
  if(color_ != other.color_) return false;
  if(camera_ != other.camera_) return false;
  if(isSample_ != other.isSample_) return false;
  if(centerPoints_ != other.centerPoints_) return false;
  if(selections_.size() != other.selections_.size()) return false;
  for(int i = 0; i < selections_.size(); i++)
    if(*(selections_[i]) != *(other.selections_[i])) return false;
  return true;
}
