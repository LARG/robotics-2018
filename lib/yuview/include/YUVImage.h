#pragma once

#include <cstddef>
#include <YUYV.h>
#include <vector>
#include <opencv2/core/core.hpp>

namespace yuview {
  enum class ImageFormat {
    RGB,
    YUV444,
    YUV422
  };
  class YUVImage {
    public:
      YUVImage() = default;
      ~YUVImage();
      static inline YUVImage ReadRawBuffer(std::string source, std::size_t width, std::size_t height) {
        return YUVImage(source, width, height);
      }
      static inline YUVImage ReadSerializedObject(std::string source) {
        return YUVImage(source);
      }
      static inline YUVImage CreateFromRawBuffer(const uint8_t* buffer, int width, int height) {
        auto image = YUVImage();
        image.raw_ = true; 
        image.width_ = width; 
        image.height_ = height;
        image.raw_buffer_ = buffer;
        return image;
      }
      static inline YUVImage CreateFromBuffer(const std::vector<uint8_t>& buffer, int width, int height) {
        auto image = YUVImage();
        image.buffer_ = buffer;
        image.width_ = width;
        image.height_ = height;
        return image;
      }
      static inline YUVImage FromMat(const cv::Mat& mat) {
        auto image = YUVImage();
        image.width_ = mat.cols;
        image.height_ = mat.rows;
        image.buffer_.resize(mat.total() * 2);
        auto buffer = image.buffer_.data();
        for(int j = 0; j < mat.rows; j++)
          for(int i = 0; i < mat.cols; i++) {
            auto p = mat.at<cv::Vec3b>(j,i);
            *(buffer++) = p[0];
            if(i % 2) *(buffer++) = p[2];
            else *(buffer++) = p[1];
          }
        return image;
      }

      
      inline std::size_t width() const { return width_; }
      inline std::size_t height() const { return height_; }
      inline std::size_t imageSize() const { return width_ * height_; }
      inline std::size_t dataSize() const { return imageSize() * 2; }
      inline const uint8_t* buffer() const { 
        if(raw_) return raw_buffer_; 
        else return buffer_.data(); 
      }
      
      void convert(std::string target, std::size_t width, std::size_t height);
      void convert(std::string target);
      void save(std::string target);
      void load(std::string source);
      void show();
      YUYV read(std::size_t x, std::size_t y) const;
      inline std::array<RGB,2> readRgb(std::size_t x, std::size_t y) const {
        auto yuv = read(x,y);
        return yuv.toRgb();
      }
      static bool verify(std::string source);
      cv::Mat toMat(ImageFormat format = ImageFormat::RGB);
    private:
      YUVImage(std::string source);
      YUVImage(std::string source, std::size_t width, std::size_t height);
      std::vector<uint8_t> buffer_;
      const uint8_t* raw_buffer_;
      std::string source_;
      std::size_t width_, height_;
      bool raw_ = false;
  };
}
