#pragma once

#include <QImage>
#include <yuview/YUVImage.h>
#include <common/Util.h>

namespace util {
  QImage yuvToQ(const yuview::YUVImage& yuv);
  yuview::YUVImage qToYUV(const QImage& q);
  QImage loadImage(QString path);
  void saveImage(const QImage& q, QString path);
}
