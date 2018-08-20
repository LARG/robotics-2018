#include <common/ColorConversion.h>

cv::Mat color::rawToMat(const unsigned char* imgraw, const ImageParams& params) {
  cv::Mat cvimage(params.height, params.width, CV_8UC3);
  for(int r = 0; r < params.height; r++) {
    for(int c = 0; c < params.width; c += 2) {
      color::Yuv422 yuyv;
      yuyv.y0 = (int) (*(imgraw++));
      yuyv.u = (int) (*(imgraw++));
      yuyv.y1 = (int) (*(imgraw++));
      yuyv.v = (int) (*(imgraw++));

      color::Rgb rgb1, rgb2;
      color::yuv422ToRgb(yuyv, rgb1, rgb2);
      cv::Vec3b color1(rgb1.b, rgb1.g, rgb1.r), color2(rgb2.b, rgb2.g, rgb2.r);
      cvimage.at<cv::Vec3b>(r, c) = color1;
      cvimage.at<cv::Vec3b>(r, c + 1) = color2;
    }
  }
  return cvimage;
}


cv::Mat color::rawToMatSubset(const unsigned char* imgraw, const ImageParams& params, int row, int col, int width, int height, int hstep, int vstep) {
  cv::Mat cvimage(height, width, CV_8UC3);

  // Usually its 4, but since there are actually only 1 set of yuv values for every 4 pixels, its times 2.

  // Raw image is stored as a char array. 4 entries in this char array give the 

  imgraw += ((2 * row * params.width) + (2 * col));

  for(int r = 0; r < height; r++) {
    for(int c = 0; c < width; c += 2) {
      color::Yuv422 yuyv;
      yuyv.y0 = (int) (*(imgraw++));
      yuyv.u = (int) (*(imgraw++));
      yuyv.y1 = (int) (*(imgraw++));
      yuyv.v = (int) (*(imgraw++));

      color::Rgb rgb1, rgb2;
      color::yuv422ToRgb(yuyv, rgb1, rgb2);
      cv::Vec3b color1(rgb1.b, rgb1.g, rgb1.r), color2(rgb2.b, rgb2.g, rgb2.r);
      cvimage.at<cv::Vec3b>(r, c) = color1;
      cvimage.at<cv::Vec3b>(r, c + 1) = color2;
    
//      imgraw += (2 * (hstep - 1));  

    
    }
    imgraw+= ((params.width - width) * 2);

//    imgraw += (2 * params.width * (vstep - 1));

  }
  return cvimage;
}

/*
 * The parameters (row, col, width, height) are all with respect to the original non-subsampled image.
 * So the returned cv::Mat will be of dimensions (height/vstep, width/hstep).
 */
cv::Mat color::rawToMatGraySubset(const unsigned char* imgraw, const ImageParams& params, int row, int col, int width, int height, int hstep, int vstep) {
  cv::Mat mat(height/vstep, width/hstep, CV_8UC1);
  const unsigned char* img_ptr;

  // indices r and c are indices of the cv::Mat, not of imgraw.
  for(int r = 0; r < height/vstep; ++r) {
    img_ptr = imgraw + 2*((row + r*vstep)*params.width + col);
    for(int c = 0; c < width/hstep; ++c) {
      mat.at<unsigned char>(r, c) = *img_ptr;
      img_ptr += 2*hstep;
    }
  }

  return mat;
}

void color::matToRaw(const cv::Mat& mat, unsigned char* imgraw, const ImageParams& iparams) {
  for(int r = 0; r < iparams.height; r++) {
    for(int c = 0; c < iparams.width; c += 2) {
      color::Rgb rgb1, rgb2;
      cv::Vec3b p1 = mat.at<cv::Vec3b>(r, c), p2 = mat.at<cv::Vec3b>(r, c + 1);
      rgb1.r = p1[2]; rgb1.g = p1[1]; rgb1.b = p1[0];
      rgb2.r = p2[2]; rgb2.g = p2[1]; rgb2.b = p2[0];
      color::Yuv422 yuv = color::rgbtoToYuv422(rgb1, rgb2);
      *(imgraw++) = yuv.y0;
      *(imgraw++) = yuv.u;
      *(imgraw++) = yuv.y1;
      *(imgraw++) = yuv.v;
    }
  }
}
