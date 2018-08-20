#ifndef FOCUS_AREA_H
#define FOCUS_AREA_H

#include <common/RobotInfo.h>
#include <common/ImageParams.h>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <algorithm>

struct FocusArea {
  typedef bool (*MergePredicate)(const FocusArea& left, const FocusArea& right);
  
  int x1, y1, x2, y2;
  int width, height, cx, cy, area;
  
  inline int distance(const FocusArea& other) const {
    int xdist;
    if(x1 > other.x1 && x1 < other.x2) xdist = 0;
    else if (other.x1 > x1 && other.x1 < x2) xdist = 0;
    else xdist = std::min(x1 > other.x1 ? x1 - other.x1 : other.x1 - x1, x2 > other.x2 ? x2 - other.x2 : other.x2 - x2);
    int ydist;
    if(y1 > other.y1 && y1 < other.y2) ydist = 0;
    else if (other.y1 > y1 && other.y1 < y2) ydist = 0;
    else ydist = std::min(y1 > other.y1 ? y1 - other.y1 : other.y1 - y1, y2 > other.y2 ? y2 - other.y2 : other.y2 - y2);
    int d = xdist + ydist; // use L1 distance because why not
    return d;
  }
  FocusArea(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {
    init();
  }
  void init() {
    width = x2 - x1;
    cx = x1 + width / 2;
    height = y2 - y1;
    cy = y1 + height / 2;
    area = width * height;
  }
  inline void merge(const FocusArea& other) {
    x1 = std::min(x1, other.x1);
    y1 = std::min(y1, other.y1);
    x2 = std::max(x2, other.x2);
    y2 = std::max(y2, other.y2);
    init();
  }
  void print() const {
    printf("f area: (%i,%i) -> (%i,%i), w=%i, h=%i, cx=%i, cy=%i\n", x1, y1, x2, y2, width, height, cx, cy);
  }

  static bool standardMergePredicate(const FocusArea& left, const FocusArea& right);
  static bool verticalMergePredicate(const FocusArea& left, const FocusArea& right);
  static std::vector<FocusArea> merge(const std::vector<FocusArea>& areas);
  static std::vector<FocusArea> mergeVertical(const std::vector<FocusArea>& areas, const ImageParams& iparams);
  static std::vector<FocusArea> merge(const std::vector<FocusArea>& areas, MergePredicate predicate);
};

#endif
