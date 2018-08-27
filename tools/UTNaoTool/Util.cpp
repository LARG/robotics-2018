#include <tool/Util.h>
#include <QColor>
#include <stdexcept>

namespace util {
  using namespace yuview;

  QImage yuvToQ(const YUVImage& yuv) {
    QImage image(yuv.width(), yuv.height(), QImage::Format_RGB888);
    for(int y = 0; y < yuv.height(); y++) {
      for(int x = 0; x < yuv.width(); x+=2) {
        auto pixels = yuv.readRgb(x, y);
        image.setPixel(x, y, qRgb(pixels[0].r, pixels[0].g, pixels[0].b));
        image.setPixel(x + 1, y, qRgb(pixels[1].r, pixels[1].g, pixels[1].b));
      }
    }
    return image;
  }

  YUVImage qToYUV(const QImage& q) {
    int width = q.width(), height = q.height();
    if(width % 2) width -= 1;
    if(height % 2) height -= 1;
    if(width <= 0 || height <= 0) {
      throw std::runtime_error("Invalid width or height.");
    }
    std::vector<uint8_t> buffer(width * height * 2);
    auto b = buffer.data();
    for(int y = 0; y < height; y++) {
      for(int x = 0; x < width; x += 2) {
        QRgb rgb0 = q.pixel(x, y);
        QRgb rgb1 = q.pixel(x + 1, y);
        YUYV yuyv;
        yuyv.fromRgb({RGB(qRed(rgb0), qGreen(rgb0), qBlue(rgb0)), RGB(qRed(rgb1), qGreen(rgb1), qBlue(rgb1))});
        *(b++) = yuyv.y0;
        *(b++) = yuyv.u;
        *(b++) = yuyv.y1;
        *(b++) = yuyv.v;
      }
    }
    return YUVImage::CreateFromBuffer(buffer, width, height);
  }

  QImage loadImage(QString path) {
    if(path.endsWith(".yuv")) {
      auto yuv = YUVImage::ReadSerializedObject(path.toStdString());
      return util::yuvToQ(yuv);
    } else {
      QImage q;
      q.load(path);
      return q;
    }
  }

  void saveImage(const QImage& q, QString path) {
    if(path.endsWith(".yuv")) {
      auto yuv = qToYUV(q);
      yuv.save(path.toStdString());
    } else {
      q.save(path);
    }
  }
}
