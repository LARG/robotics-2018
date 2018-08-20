#include <vision/structures/FocusArea.h>


bool FocusArea::standardMergePredicate(const FocusArea& left, const FocusArea& right) {
  int d = left.distance(right);
  return d < 5;
}
bool FocusArea::verticalMergePredicate(const FocusArea& left, const FocusArea& right) {
  int d = left.distance(right);
  if(d < 100) return true;
  return false;
}

std::vector<FocusArea> FocusArea::merge(const std::vector<FocusArea>& areas) {
  return merge(areas, standardMergePredicate);
}

std::vector<FocusArea> FocusArea::mergeVertical(const std::vector<FocusArea>& areas, const ImageParams& iparams) {
  //printf("vert merge\n");
  const int binCount = 160, width = 3, minCount = 10;
  const int binSize = iparams.width / binCount;
  std::vector<std::vector<FocusArea> > bins; bins.resize(binCount);
  for(unsigned int i = 0; i < areas.size(); i++) {
    const FocusArea& area = areas[i];
    int n = area.cx / binSize;
    bins[n].push_back(area);
  }
  std::vector<FocusArea> all;
  for(int i = 0; i < binCount; i++) {
    int aCount = bins[i].size();
    if(aCount > minCount) {
      std::vector<FocusArea> post;
      for(int j = std::max(1, i - width); j <= std::min(binCount - 1, i + width); j++) {
        std::vector<FocusArea> column = bins[j];
        post.insert(post.end(), column.begin(), column.end());
      }
      post = merge(post, verticalMergePredicate);
      all.insert(all.end(), post.begin(), post.end());
      i += width;
    }
  }
  return all;
}

std::vector<FocusArea> FocusArea::merge(const std::vector<FocusArea>& areas, MergePredicate predicate) {
  //printf("merging %i areas\n", areas.size());
  std::vector<FocusArea> merged;
  if(areas.size() > 200 || areas.size() == 0) return areas;
  bool ismerged[areas.size()];
  memset(ismerged, false, areas.size());
  //int iter = 0;
  for(unsigned int i = 0; i < areas.size(); i++) {
    if(ismerged[i]) continue;
    FocusArea mfocus = areas[i];
    for(unsigned int j = i + 1; j < areas.size(); j++) {
      if(ismerged[j]) continue;
      //iter++;
      const FocusArea& aj = areas[j];
      if(predicate(mfocus, aj)) {
        ismerged[j] = true;
        mfocus.merge(aj);
      //} else {
        //printf("NO MERGE\n");
        //mfocus.print();
        //aj.print();
        //printf("dist: %i\n", mfocus.distance(aj));
      }
      if(mfocus.area > 200 * 200) break;
    }
    merged.push_back(mfocus);
  }
  //printf("iter: %i\n", iter);
  return merged;
}
//std::vector<FocusArea> FocusArea::merge(const std::vector<FocusArea>& areas, MergePredicate predicate) {
  //std::vector<FocusArea> merged;
  //if(areas.size() > 200 || areas.size() == 0) return areas;
  //bool ismerged[areas.size()];
  //memset(ismerged, false, areas.size());
  //for(unsigned int i = 0; i < areas.size(); i++) {
    //if(ismerged[i]) continue;
    //FocusArea mfocus = areas[i];
    //for(unsigned int j = i + 1; j < areas.size(); j++) {
      //if(ismerged[j]) continue;
      //const FocusArea& aj = areas[j];
      //if(predicate(mfocus, aj)) {
        //ismerged[j] = true;
        //mfocus.merge(aj);
      //}
      //if(mfocus.area > 200 * 200) break;
    //}
    //merged.push_back(mfocus);
  //}
  //return merged;
//}
