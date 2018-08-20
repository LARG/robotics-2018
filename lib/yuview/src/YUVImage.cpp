#include <YUVImage.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <schema/YUVFile_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <Util.h>
#include <stdint.h>
#include <fstream>

namespace yuview {

  using namespace std;

  YUVImage::YUVImage(string source) : source_(source) {
    load(source);
  }

  YUVImage::YUVImage(string source, size_t width, size_t height) : 
      source_(source), width_(width), height_(height) {
    buffer_ = Util::fread(source);
  }

  YUVImage::~YUVImage() {
#ifdef ENABLE_GUI
    cv::destroyAllWindows();
#endif
  }

  YUYV YUVImage::read(std::size_t x, std::size_t y) const {
    if(x % 2 != 0) {
      throw std::runtime_error("Invalid read request: Buffer must be read at intervals of 2 pixels.");
    }
    auto buffer = this->buffer();
    YUYV result;
    result.read(&buffer[2 * y * width() + 2 * x]);
    return result;
  }

  cv::Mat YUVImage::toMat(ImageFormat format) {
    if(format == ImageFormat::YUV422) {
      cv::Mat image(this->height(), this->width(), CV_8UC2, const_cast<uint8_t*>(this->buffer()));
      return image;
    }
    cv::Mat image(this->height(), this->width(), CV_8UC3);
    for(auto y = 0; y < this->height(); y++) {
      for(auto x = 0; x < this->width(); x += 2) {
        assert(this->imageSize() >= y * this->width() + x);
        if(format == ImageFormat::RGB) {
          auto pixels = this->readRgb(x,y);
          image.at<cv::Vec3b>(y, x) = cv::Vec3b(pixels[0].b, pixels[0].g, pixels[0].r);
          image.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(pixels[1].b, pixels[1].g, pixels[1].r);
        } else if(format == ImageFormat::YUV444) {
          auto yuyv = read(x,y);
          image.at<cv::Vec3b>(y, x) = cv::Vec3b(yuyv.y0, yuyv.u, yuyv.v);
          image.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(yuyv.y1, yuyv.u, yuyv.v);
        } 
      }
    }
    return image;
  }

  void YUVImage::convert(string target, size_t width, size_t height) {
    auto image = toMat();
#ifdef ALLOW_RESIZE
    cv::resize(image, image, cv::Size(width, height));
#endif
    cv::imwrite(target, image);
  }

  void YUVImage::convert(string target) {
    auto image = toMat();
    cv::imwrite(target, image);
  }

  void YUVImage::save(string target) {
    flatbuffers::FlatBufferBuilder builder;
    auto buffer = builder.CreateVector(this->buffer(), this->dataSize());
    auto ybuilder = YUVFileBuilder(builder);
    ybuilder.add_width(this->width());
    ybuilder.add_height(this->height());
    ybuilder.add_buffer(buffer);
    builder.Finish(ybuilder.Finish());
    ofstream out(target, ios::out | ios::binary);
    out.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
  }

  bool YUVImage::verify(string source) {
    auto buffer = Util::fread(source);
    auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
    bool ok = VerifyYUVFileBuffer(verifier);
    return ok;
  }

  void YUVImage::load(string source) {
    ifstream in(source, ios::in | ios::binary | ios::ate);
    auto buffer = Util::fread(source);
    auto file = GetYUVFile(buffer.data());
    width_ = file->width();
    height_ = file->height();
    buffer_.resize(file->buffer()->size());
    copy(file->buffer()->begin(), file->buffer()->end(), buffer_.begin());
  }

  void YUVImage::show() {
#ifdef ENABLE_GUI
    cv::destroyAllWindows();
    auto image = toMat();
    cv::namedWindow(source_, cv::WINDOW_AUTOSIZE);
    cv::imshow(source_, image);
#endif
  }
}
