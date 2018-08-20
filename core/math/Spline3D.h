#ifndef SPLINE3D_IY189LJZ
#define SPLINE3D_IY189LJZ

#include <iostream>
#include <../lib/alglib/ap.h>
#include <../lib/alglib/interpolation.h>
#include <math/Geometry.h>
#include <math/Vector3.h>

class Spline3D {
public:
  Spline3D() {initialized = false;}

  void set(int num_pts, double times[], double xs[], double ys[], double zs[], bool use_akima_spline) {
    if (num_pts < 5) {
      std::cerr << "ERROR: Spline3D::set - too few points " << num_pts << ", need at least 5" << std::endl;
      return;
    }

    num_pts_ = num_pts;
    use_akima_spline_ = use_akima_spline;
    time_arr_.setcontent(num_pts,times);
    x_arr_.setcontent(num_pts,xs);
    y_arr_.setcontent(num_pts,ys);
    z_arr_.setcontent(num_pts,zs);

    createSplines();
    initialized = true;
  }

  void calc(float time, Vector3<float> &vals) {
    vals.x = alglib::spline1dcalc(x_spline_,time);
    vals.y = alglib::spline1dcalc(y_spline_,time);
    vals.z = alglib::spline1dcalc(z_spline_,time);
  }

  void setPoint(int ind, float x, float y) {
    x_arr_[ind] = x;
    y_arr_[ind] = y;
  }

  void scaleTime(float start, float end, float scale) {
    float start_frac;
    float end_frac;
    float total_frac;
    float range;

    float offset = 0;
    for (int i = 0; i < num_pts_ - 1; i++) {
      range = time_arr_[i + 1] - time_arr_[i];

      start_frac = crop((start - time_arr_[i]) / range,0,1.0);
      end_frac = crop(1.0 - (time_arr_[i+1] - end) / range,0,1.0);
      total_frac = crop(end_frac - start_frac,0,1.0);

      time_arr_[i] += offset;
      offset += total_frac * range * (scale - 1.0);
    }
    time_arr_[num_pts_-1] += offset;
  }

  void rotatePoints(int startInd, int endInd, float ang, int rotateMode) {
    double temp_x;
    for (int i = startInd; i <= endInd; i++) {
      temp_x = x_arr_[i];
      if (rotateMode == 0)
        x_arr_[i] = temp_x * cos(ang) - y_arr_[i] * sin(ang);
      if (rotateMode <= 1)
        y_arr_[i] = temp_x * sin(ang) + y_arr_[i] * cos(ang);
      if (rotateMode == 2)
        y_arr_[i] = temp_x * tan(ang);
    }
  }

  void setX(int ind, float x) {
    x_arr_[ind] = x;
  }

  void setOffset(int startPt, int endPt, float amount, int dim, bool doCreateSplines = true) {
    alglib::real_1d_array *arr = getArray(dim);

    for (int i = startPt; i <= endPt; i++)
      (*arr)[i] += amount;

    if (doCreateSplines)
      createSplines();
  }

  void cropVals(int dim, float minVal, float maxVal) {
    alglib::real_1d_array *arr = getArray(dim);
    for (int i = 0; i < num_pts_; i++) {
      //std::cout << "orig[" << i << "] = " << (*arr)[i] << std::endl;
      (*arr)[i] = crop((*arr)[i],minVal,maxVal);
      //std::cout << "new [" << i << "] = " << (*arr)[i] << std::endl;
    }
  }

  float totalTime() {
    return time_arr_[num_pts_-1];
  }

  bool isInitialized() const {return initialized;}

  void createSplines() {
    createSpline(&x_arr_,&x_spline_);
    createSpline(&y_arr_,&y_spline_);
    createSpline(&z_arr_,&z_spline_);
  }

  Spline3D& operator=(const Spline3D &other) {
    num_pts_ = other.num_pts_;
    if (!other.isInitialized() || (num_pts_ < 5)) {
      //std::cerr << "ERROR: Spline3D::operator= - not initialized or too few points " << num_pts_ << ", need at least 5" << std::endl;
      initialized = false;
      return *this;
    }
    use_akima_spline_ = other.use_akima_spline_;
    time_arr_.setcontent(num_pts_,other.time_arr_.getcontent());
    x_arr_.setcontent(num_pts_,other.x_arr_.getcontent());
    y_arr_.setcontent(num_pts_,other.y_arr_.getcontent());
    z_arr_.setcontent(num_pts_,other.z_arr_.getcontent());
    createSplines();
    initialized = true;
    return *this;
  }

private:
  alglib::real_1d_array* getArray(int dim) {
    switch (dim) {
      case 0:
        return &x_arr_;
        break;
      case 1:
        return &y_arr_;
        break;
      case 2:
        return &z_arr_;
        break;
      case 3:
        return &time_arr_;
        break;
      default:
        std::cout << "Spline3D::getArray, bad dim: " << dim << std::endl;
        return &x_arr_;
    }
  }

  int num_pts_;
  bool use_akima_spline_;

  alglib::spline1dinterpolant x_spline_;
  alglib::spline1dinterpolant y_spline_;
  alglib::spline1dinterpolant z_spline_;
public:
  alglib::real_1d_array time_arr_;
  alglib::real_1d_array x_arr_;
  alglib::real_1d_array y_arr_;
  alglib::real_1d_array z_arr_;

  bool initialized;

  void createSpline(alglib::real_1d_array *arr, alglib::spline1dinterpolant *spline) {
    if (use_akima_spline_)
      alglib::spline1dbuildakima(time_arr_,*arr,*spline);
    else
      alglib::spline1dbuildcubic(time_arr_,*arr,*spline);
  }
};

#endif /* end of include guard: SPLINE3D_IY189LJZ */
