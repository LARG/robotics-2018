#include "AnnotationAnalyzer.h"

AnnotationAnalyzer::AnnotationAnalyzer() : table_(0) {
}

void AnnotationAnalyzer::setAnnotations(std::vector<VisionAnnotation*> annotations){
  annotations_ = annotations;
}

void AnnotationAnalyzer::setImages(ImageList images, std::vector<ImageParams> iparams){
  //if(images_.size() > 0)
  //  for(auto image : images_) delete image;
  images_ = images;
  iparams_ = iparams;
}

void AnnotationAnalyzer::setColorTable(ColorTable table) {
  table_ = table;
}

float AnnotationAnalyzer::falsePositiveRate(Color query){
  int totalEnclosed = 0;
  for(unsigned int i = 0; i < annotations_.size(); i++){
    VisionAnnotation* annotation = annotations_[i];
    if(annotation->getColor() != query) continue;
    int enclosedCount = annotation->getEnclosedPoints().size();
    for(unsigned int j = 0; j < images_.size(); j++){
      if(!annotation->isInFrame(j)) continue;
      totalEnclosed += enclosedCount;
    }
  }
  if(totalEnclosed == 0)
    return 0;
  return (float)falsePositiveCount(query) / totalEnclosed;
}

int AnnotationAnalyzer::falsePositiveCount(Color query){
  return falsePositives(query).size();
}

std::vector<Point> AnnotationAnalyzer::falsePositives(Color query, int frame) {
  std::vector<Point> points;
  if(!table_) return points;
  unsigned char* image = images_[frame].data();
  const ImageParams& iparams = iparams_[frame];
  for(int x = 0; x < iparams.width; x++){
    for(int y = 0; y < iparams.height; y++) {
      Color c = ColorTableMethods::xy2color(image, table_, x, y, iparams.width);
      if(c == query) {
        bool enclosed = false;
        for(unsigned int j = 0; j < annotations_.size(); j++){
          VisionAnnotation* annotation = annotations_[j];
          if(annotation->getColor() != query) continue;
          if(!annotation->isInFrame(frame)) continue;
          if(annotation->enclosesPoint(frame,x,y)) {
            enclosed = true;
            break;
          }
        }
        if(!enclosed) points.push_back(Point(x,y));
      }
    }
  }
  return points;
}

std::vector<Point> AnnotationAnalyzer::falsePositives(Color query) {
  std::vector<Point> points;
  if(!table_) return points;
  for(uint16_t i = 0; i < images_.size(); i++) {
    std::vector<Point> framePoints = falsePositives(query, i);
    uint16_t fcount = framePoints.size();
    for(uint16_t j = 0; j < fcount; j++)
      points.push_back(framePoints[j]);
  }
  return points;
}

std::vector<Point> AnnotationAnalyzer::truePositives(Color query, int frame) {
  std::vector<Point> points;
  if(!table_) return points;
  unsigned char* image = images_[frame].data();
  const ImageParams& iparams = iparams_[frame];
  for(unsigned int i = 0; i < annotations_.size(); i++){
    VisionAnnotation* annotation = annotations_[i];
    if(annotation->getColor() != query) continue;
    if(!annotation->isInFrame(frame)) continue;
    std::vector<Point> enclosed = annotation->getEnclosedPoints(frame);
    int count = enclosed.size();
    for(int j = 0 ; j < count; j++) {
      Point p = enclosed[j];
      if(p.x >= iparams.width || p.y >= iparams.height) continue;
      Color c = ColorTableMethods::xy2color(image, table_, p.x, p.y, iparams.width);
      if(c == query){
        points.push_back(p);
      }
    }
  }
  return points;
}

std::vector<Point> AnnotationAnalyzer::truePositives(Color query) {
  std::vector<Point> points;
  if(!table_) return points;
  for(uint16_t i = 0; i < images_.size(); i++) {
    std::vector<Point> framePoints = truePositives(query, i);
    uint16_t fcount = framePoints.size();
    for(uint16_t j = 0; j < fcount; j++)
      points.push_back(framePoints[j]);
  }
  return points;
}

float AnnotationAnalyzer::falseNegativeRate(Color query){
  int totalEnclosed = 0;
  for(unsigned int i = 0; i < annotations_.size(); i++){
    VisionAnnotation* annotation = annotations_[i];
    if(annotation->getColor() != query) continue;
    int enclosedCount = annotation->getEnclosedPoints().size();
    for(unsigned int j = 0; j < images_.size(); j++){
      if(!annotation->isInFrame(j)) continue;
      totalEnclosed += enclosedCount;
    }
  }
  if(totalEnclosed == 0)
    return 0;
  return (float)falseNegativeCount(query) / totalEnclosed;
}

int AnnotationAnalyzer::falseNegativeCount(Color query){
  if(!table_) return 0;
  int fn = 0;
  for(unsigned int i = 0; i < annotations_.size(); i++){
    VisionAnnotation* annotation = annotations_[i];
    if(annotation->getColor() != query) continue;
    for(unsigned int frame = 0; frame < images_.size(); frame++){
      if(!annotation->isInFrame(frame)) continue;
      unsigned char* image = images_[frame].data();
      const ImageParams& iparams = iparams_[frame];
      std::vector<Point> points = annotation->getEnclosedPoints(frame);
      int count = points.size();
      for(int j = 0; j < count; j++) {
        Point p = points[j];
        if(p.x >= iparams.width || p.y >= iparams.height) continue;
        Color c = ColorTableMethods::xy2color(image,table_,p.x,p.y,iparams.width);
        if(c != query) fn++;
      }
    }
  }
  return fn;
}

int AnnotationAnalyzer::colorTablePointCount(Color query) {
  if(!table_) return 0;
  int count = 0;
  for(int y = 0; y < 256; y+=2) {
    for(int u = 0; u < 256; u+=2) {
      for(int v = 0; v < 256; v+=2) {
        if(ColorTableMethods::yuv2color(table_,y,u,v) == query){
          count++;
        }
      }
    }
  }
  return count;
}

std::vector<YUV*> AnnotationAnalyzer::getCriticalPoints(Color query) {
  std::vector<YUV*> points;
  if(pruningCache_.find(query) != pruningCache_.end()) {
    std::list<YUV*>& lpoints = pruningCache_[query];
    for(std::list<YUV*>::iterator it = lpoints.begin(); it != lpoints.end(); it++) {
      points.push_back(*it);
    }
    return points;
  }
  if(!table_) return points;
  pruningCache_[query] = std::list<YUV*>();
  memset(fpmap_, 0, LUT_SIZE);
  for(int y = 0; y < 256; y+=2) {
    for(int u = 0; u < 256; u+=2) {
      for(int v = 0; v < 256; v+=2) {
        if(ColorTableMethods::yuv2color(table_,y,u,v) == query){
          YUV* yuv = new YUV(y,u,v,query);
          points.push_back(yuv);
          *(fpmap_ + ((y >> 1 << 14) + (u >> 1 << 7) + (v >> 1))) = yuv;
        }
      }
    }
  }
  for(uint16_t i = 0; i < images_.size(); i++) {
    unsigned char* image = images_[i].data();
    const ImageParams& iparams = iparams_[i];
    std::vector<Point> fps = falsePositives(query,i);
    int count = fps.size();
    for(int j = 0; j < count; j++){
      Point p = fps[j];
      int y, u, v;
      if(p.x >= iparams.width || p.y >= iparams.height) continue;
      ColorTableMethods::xy2yuv(image,p.x,p.y,iparams.width, y,u,v);
      YUV* yuv = *(fpmap_ + ((y >> 1 << 14) + (u >> 1 << 7) + (v >> 1)));
      yuv->fpcount++;
    }
    std::vector<Point> tps = truePositives(query, i);
    count  = tps.size();
    for(int j = 0; j < count; j++){
      Point p = tps[j];
      int y,u,v;
      if(p.x >= iparams.width || p.y >= iparams.height) continue;
      ColorTableMethods::xy2yuv(image,p.x,p.y,iparams.width, y,u,v);
      YUV* yuv = *(fpmap_ + ((y >> 1 << 14) + (u >> 1 << 7) + (v >> 1)));
      yuv->tpcount++;
    }
  }
  int pcount = points.size();
  for(int i = 0; i < pcount; i++)
    points[i]->setScore();
  sort (points.begin(), points.end(), YUV::sortPredicate);
  for(int i = 0; i < pcount; i++)
    pruningCache_[query].push_back(points[i]);
  return points;
}

void AnnotationAnalyzer::removeCriticalPoints(Color query, float percentage) {
  if(!table_) return;
  std::vector<YUV*> points = getCriticalPoints(query);
  std::vector<YUV*> removed;
  bool remove = true;
  float total = points.size();
  for(int i = 0; i < total; i++) {
    float proportion = i / total;
    if(proportion > percentage)
      remove = false;
    YUV* point = points[i];
    if(remove) {
      ColorTableMethods::assignColor(table_, point->y, point->u, point->v, c_UNDEFINED);
      removed.push_back(point);
      pruningCache_[query].pop_front();
    }
  }
  pruningStack_.push_back(removed);
}

void AnnotationAnalyzer::undo() {
  if(!table_) return;
  if(pruningStack_.size() == 0) return;

  std::vector<YUV*> last = pruningStack_.back();
  pruningStack_.pop_back();
  int total = last.size();
  for(int i = total - 1; i >= 0; i--) {
    YUV* point = last[i];
    ColorTableMethods::assignColor(table_, point->y, point->u, point->v, point->color);
    pruningCache_[point->color].push_front(point);
  }
}

void AnnotationAnalyzer::clear(){
  for(uint16_t i = 0; i < pruningStack_.size(); i++){
    std::vector<YUV*> current = pruningStack_[i];
    uint16_t count = current.size();
    for(uint16_t j = 0; j < count; j++) {
      delete current[j];
    }
  }
  pruningStack_.clear();
  for(int i = 0; i < NUM_Colors; i++) {
    Color c = (Color)i;
    if(pruningCache_.find(c) != pruningCache_.end()) {
      std::list<YUV*> points = pruningCache_[c];
      for(std::list<YUV*>::iterator it = pruningCache_[c].begin(); it != pruningCache_[c].end(); it++) {
        YUV* yuv = *it;
        delete yuv;
      }
    }
  }
  pruningCache_.clear();
}

bool AnnotationAnalyzer::hasUndo() {
  return pruningStack_.size() > 0;
}
