#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <math/Point.h>
#include <list>
#include <vector>

#include <memory/TextLogger.h>
//#include <vision/ImageConstants.h>
#include <vision/VisionConstants.h>
#include <vision/structures/VisionPoint.h>
#include <vision/structures/VisionParams.h>
#include <vision/structures/HorizonLine.h>
#include <vision/enums/Colors.h>
#include <vision/ColorTableMethods.h>
#include <vision/VisionBlocks.h>
#include <vision/structures/FocusArea.h>
#include <vision/Macros.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <common/Profiling.h>

/// @ingroup vision
class Classifier {
 public:
  Classifier(const VisionBlocks& vblocks, const VisionParams& vparams, const ImageParams& iparams, const Camera::Type& camera);
  ~Classifier();
  void init(TextLogger* tl){textlogger = tl;};

  bool classifyImage(unsigned char*);
  inline Color xy2color(int x, int y) {
    return (Color)segImg_[y * iparams_.width + x];
  }
  void getStepSize(int&,int&) const;

 private:
  void classifyImage(const FocusArea& area, unsigned char*);
  bool setImagePointers();
  
  const VisionBlocks& vblocks_;
  const VisionParams& vparams_;
  const ImageParams& iparams_;
  const Camera::Type& camera_;
  bool initialized_;
  TextLogger* textlogger;

  unsigned char* img_;
  unsigned char* segImg_, *segImgLocal_;
  HorizonLine horizon_;
  unsigned char* colorTable_;
};
#endif
