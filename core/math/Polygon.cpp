#include "Polygon.h"

Polygon::Polygon() {
    count_ = 0;
}

Polygon::Polygon(std::vector<Point> vertices) : vertices_(vertices){
    count_ = vertices.size();
    xcoords_ = new int[count_];
    ycoords_ = new int[count_];
    for(int i = 0; i < count_; i++) {
        xcoords_[i] = vertices_[i].x;
        ycoords_[i] = vertices_[i].y;
    }
    setBounds();
}

void Polygon::addVertex(int x, int y) {
    vertices_.push_back(Point(x,y));
    count_++;
    xcoords_ = new int[count_];
    ycoords_ = new int[count_];
    for(int i = 0; i < count_; i++) {
        xcoords_[i] = vertices_[i].x;
        ycoords_[i] = vertices_[i].y;
    }
    setBounds();
}

/** From: http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html **/
bool Polygon::enclosesPoint(int nvert, int *vertx, int *verty, int testx, int testy) {
  int i, j;
  bool c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((verty[i]>testy) != (verty[j]>testy)) &&
	 (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
       c = !c;
  }
  return c;
}

int Polygon::getTop() {
    if(count_ == 0)
        return 0;
    int top = 0;
    for(int i=0;i<count_;i++)
        if(top < vertices_[i].y)
            top = vertices_[i].y;
    return top;
}

int Polygon::getBottom() {
    if(count_ == 0)
        return 0;
    int bottom = 0;
    for(int i=0;i<count_;i++)
        if(bottom > vertices_[i].y)
            bottom = vertices_[i].y;
    return bottom;
}

int Polygon::getLeft() {
    if(count_ == 0)
        return 0;
    int left = 0;
    for(int i=0;i<count_;i++)
        if(left > vertices_[i].x)
            left = vertices_[i].x;
    return left;
}

int Polygon::getRight() {
    if(count_ == 0)
        return 0;
    int right = 0;
    for(int i=0;i<count_;i++)
        if(right < vertices_[i].x)
            right = vertices_[i].x;
    return right;
}

std::vector<Point> Polygon::enclosedPoints() {
    if(enclosed_.size() > 0) return enclosed_;
    // Only look through the region in the bounding box to minimize iterations
    int top = getTop(), bottom = getBottom(), left = getLeft(), right = getRight();
    for(int i=left; i <= right; i++){
        for(int j=bottom; j <= top; j++){
            if(enclosesPoint(i,j))
                enclosed_.push_back(Point(i,j));
        }
    }
    return enclosed_;
}

bool Polygon::enclosesPoint(int x, int y){
    if(x < left_ || x > right_ || y < bottom_ || y > top_) return false; // Ignore points outside the bounding box to improve performance
    int lookup = (x << 16) | y; // This assumes that the image dimensions are less than 2^16
    std::map<int,bool>::iterator it = pointCache_.find(lookup);
    bool value;
    if(it == pointCache_.end()) {
        bool value = enclosesPoint(count_, xcoords_, ycoords_, x, y);
        pointCache_[lookup] = value;
        return value;
    } else return it->second;
}

void Polygon::setBounds() {
    left_ = getLeft();
    top_ = getTop();
    right_ = getRight();
    bottom_ = getBottom();
}
