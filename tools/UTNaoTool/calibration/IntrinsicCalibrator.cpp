#ifdef ENABLE_OPENCV
#include "IntrinsicCalibrator.h"

#define DOT(v,w) ((v).x * (w).x + (v).y * (w).y)
#define NORM(v) (sqrtf((v).x * (v).x + (v).y * (v).y))
#define ANGLE(a,b,c) acosf(DOT(a - b, c - b) / (NORM(a - b) * NORM(c - b)))
#define PARAM_DISTANCE(p,q) (abs(p.x - q.x) + abs(p.y - q.y) + abs(p.size - q.size) + abs(p.skew - q.skew))
#define PRINT_PARAMS(p) printf("x: %2.4f, y: %2.4f, size: %2.4f, skew: %2.4f\n", p.x, p.y, p.size, p.skew)
#define SET_PARAMS(p, v) p.x = p.y = p.size = p.skew = v
#define LMIN_PARAMS(p, q) do { if(p.x > q.x) p.x = q.x; if(p.y > q.y) p.y = q.y; if(p.size > q.size) p.size = q.size; if(p.skew > q.skew) p.skew = q.skew; } while(0)
#define LMAX_PARAMS(p, q) do { if(p.x < q.x) p.x = q.x; if(p.y < q.y) p.y = q.y; if(p.size < q.size) p.size = q.size; if(p.skew < q.skew) p.skew = q.skew; } while(0)


using namespace cv;
using namespace std;

IntrinsicCalibrator::IntrinsicCalibrator(ICSettings settings) : settings_(settings) {
}

ICMeasures IntrinsicCalibrator::getParameters(vector<Point2f> points) {
  Point2f 
    uleft = points[0], uright = points[settings_.boardSize.width - 1],
    dleft = points[points.size() - settings_.boardSize.width], dright = points[points.size() - 1];
  Point2f
    a = uright - uleft,
    b = dright - uright,
    c = dleft - dright;
  Point2f p = b + c, q = a + b;
  float area = abs(p.x * q.y - p.y * q.x) / 2;
  float border = sqrt(area);
  float avgX = 0, avgY = 0;
  for(int i = 0; i < (int)points.size(); i++) {
    avgX += points[i].x;
    avgY += points[i].y;
  }
  avgX /= points.size(); avgY /= points.size();
  ICMeasures params;
  params.x = min(1.0f, max(0.0f, (avgX - border / 2) / (settings_.imageSize.width - border)));
  params.y = min(1.0f, max(0.0f, (avgY - border / 2) / (settings_.imageSize.height - border)));
  params.size = sqrt(area / (settings_.imageSize.width * settings_.imageSize.height));
  params.skew  = min(1.0f, 2.0f * (float)abs((M_PI / 2.0f) - ANGLE(uleft, uright, dright)));
  //printf("sample: "); PRINT_PARAMS(params);
  return params;
}

ICMeasures IntrinsicCalibrator::getProgress() {
  ICMeasures progress;
  if(sampleParams_.size() == 0) {
    progress.x = progress.y = progress.size = progress.skew = 0;
    return progress;
  }
  ICMeasures minP, maxP;
  SET_PARAMS(minP, FLT_MAX);
  SET_PARAMS(maxP, 0.0f);
  for(int i = 0; i < (int)sampleParams_.size(); i++) {
    ICMeasures& sp = sampleParams_[i];
    LMIN_PARAMS(minP, sp);
    LMAX_PARAMS(maxP, sp);
  }
  minP.size = minP.skew = 0.0f;
  progress.x = min(1.f, (maxP.x - minP.x) / .7f);
  progress.y = min(1.f, (maxP.y - minP.y) / .7f);
  progress.size = min(1.f, (maxP.size - minP.size) / .4f);
  progress.skew = min(1.f, (maxP.skew - minP.skew) / .5f);
  return progress;
}

int IntrinsicCalibrator::getSampleCount() {
  return imagePoints_.size();
}

bool IntrinsicCalibrator::isGoodSample(ICMeasures params) {
  if(sampleParams_.size() == 0) return true;
  float minDist = FLT_MAX;
  for(int i = 0; i < (int)sampleParams_.size(); i++) {
    ICMeasures& sp = sampleParams_[i];
    float dist = PARAM_DISTANCE(sp, params);
    if(dist < minDist)
      minDist = dist;
  }
  return minDist > 0.2f;
}

vector<Point2f> IntrinsicCalibrator::addImage(const cv::Mat& image) {
  Mat grayImage;
  cvtColor(image, grayImage, CV_BGR2GRAY);
  images_.push_back(image);
  vector<Point2f> imagePoints;
  bool found = cv::findChessboardCorners(image, settings_.boardSize, imagePoints,
      CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

  if(settings_.patternType == CHESSBOARD && found) {
    cv::cornerSubPix(grayImage, imagePoints, Size(11,11), Size(-1, -1),
        TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
  }

  if(found) {
    ICMeasures params = getParameters(imagePoints);
    if(isGoodSample(params)) {
      sampleParams_.push_back(params);
      imagePoints_.push_back(imagePoints);
      ICMeasures progress = getProgress();
      printf("sample accepted: "); PRINT_PARAMS(params);
    }
  }
  return imagePoints;
}

void IntrinsicCalibrator::addImages(const vector<Mat>& images) {
  for(int i = 0; i < (int)images.size(); i++) {
    //printf("Image %03i\n", i);
    //printf("Processing image %03i...\r", i);
    //std::cout.flush();
    const Mat& image = images[i];
    addImage(image);
  }
}

double IntrinsicCalibrator::computeReprojectionErrors(
        const vector<Mat>& rvecs, const vector<Mat>& tvecs,
        vector<float>& perViewErrors) const
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints_.size());
    
    for( i = 0; i < (int)objectPoints_.size(); i++ )
    {
        projectPoints(Mat(objectPoints_[i]), rvecs[i], tvecs[i],
                      cameraMatrix_, distortionCoeffs_, imagePoints2);
        err = norm(Mat(imagePoints_[i]), Mat(imagePoints2), CV_L2);
        int n = (int)objectPoints_[i].size();
        perViewErrors[i] = (float)std::sqrt(err*err/n);
        totalErr += err*err;
        totalPoints += n;
    }
    
    return std::sqrt(totalErr/totalPoints);
}

vector<Point3f> IntrinsicCalibrator::calcChessboardCorners() const {
  vector<Point3f> corners;
  cv::Size boardSize = settings_.boardSize;
  float squareSize = settings_.squareSize;
  switch(settings_.patternType) {
    case CHESSBOARD:
    case CIRCLES_GRID:
      for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
          corners.push_back(Point3f(float(j*squareSize),
                float(i*squareSize), 0));
      break;

    case ASYMMETRIC_CIRCLES_GRID:
      for( int i = 0; i < boardSize.height; i++ )
        for( int j = 0; j < boardSize.width; j++ )
          corners.push_back(Point3f(float((2*j + i % 2)*squareSize),
                float(i*squareSize), 0));
      break;

    default:
      CV_Error(CV_StsBadArg, "Unknown pattern type\n");
  }
  return corners;
}

bool IntrinsicCalibrator::runCalibration(vector<Mat>& rvecs, vector<Mat>& tvecs, vector<float>& reprojErrs, double& totalAvgErr) {
    cameraMatrix_ = Mat::eye(3, 3, CV_64F);
    if(settings_.flags & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix_.at<double>(0,0) = settings_.aspectRatio;
    
    distortionCoeffs_ = Mat::zeros(8, 1, CV_64F);

    //map<string, float> params = getParameters(imagePoints_);
    
    vector<Point3f> corners = calcChessboardCorners();

    objectPoints_.resize(imagePoints_.size(),corners);
    
    double rms = calibrateCamera(objectPoints_, imagePoints_, settings_.imageSize, cameraMatrix_,
                    distortionCoeffs_, rvecs, tvecs, settings_.flags|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
                    ///*|CV_CALIB_FIX_K3*/|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    printf("RMS error reported by calibrateCamera: %g\n", rms);
    
    bool ok = checkRange(cameraMatrix_) && checkRange(distortionCoeffs_);
    
    totalAvgErr = computeReprojectionErrors(rvecs, tvecs, reprojErrs);

    cout << "distortion:\n" << distortionCoeffs_ << "\n";

    return ok;
}


void IntrinsicCalibrator::saveCameraParams(const string& filename,
                       const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                       const vector<float>& reprojErrs,
                       double totalAvgErr, bool writePoints) const
{
  FileStorage fs( filename, FileStorage::WRITE );

  time_t t;
  time( &t );
  struct tm *t2 = localtime( &t );
  char buf[1024];
  strftime( buf, sizeof(buf)-1, "%c", t2 );

  fs << "calibration_time" << buf;

  if( !rvecs.empty() || !reprojErrs.empty() )
    fs << "nframes" << (int)std::max(rvecs.size(), reprojErrs.size());
  fs << "image_width" << settings_.imageSize.width;
  fs << "image_height" << settings_.imageSize.height;
  fs << "board_width" << settings_.boardSize.width;
  fs << "board_height" << settings_.boardSize.height;
  fs << "square_size" << settings_.squareSize;

  unsigned int flags = settings_.flags;
  if(flags & CV_CALIB_FIX_ASPECT_RATIO )
    fs << "aspectRatio" << settings_.aspectRatio;

  if(flags != 0 ) {
    sprintf( buf, "flags: %s%s%s%s",
        flags & CV_CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
        flags & CV_CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
        flags & CV_CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
        flags & CV_CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "" );
    cvWriteComment( *fs, buf, 0 );
  }

  fs << "flags" << (int)flags;

  fs << "camera_matrix" << cameraMatrix_;
  fs << "distortion_coefficients" << distortionCoeffs_;

  fs << "avg_reprojection_error" << totalAvgErr;
  if( !reprojErrs.empty() )
    fs << "per_view_reprojection_errors" << Mat(reprojErrs);

  if( !rvecs.empty() && !tvecs.empty() )
  {
    CV_Assert(rvecs[0].type() == tvecs[0].type());
    Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
    for( int i = 0; i < (int)rvecs.size(); i++ )
    {
      Mat r = bigmat(Range(i, i+1), Range(0,3));
      Mat t = bigmat(Range(i, i+1), Range(3,6));

      CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
      CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
      //*.t() is MatExpr (not Mat) so we can use assignment operator
      r = rvecs[i].t();
      t = tvecs[i].t();
    }
    cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
    fs << "extrinsic_parameters" << bigmat;
  }

  if(writePoints && !imagePoints_.empty())
  {
    Mat imagePtMat((int)imagePoints_.size(), (int)imagePoints_[0].size(), CV_32FC2);
    for( int i = 0; i < (int)imagePoints_.size(); i++ )
    {
      Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
      Mat imgpti(imagePoints_[i]);
      imgpti.copyTo(r);
    }
    fs << "image_points" << imagePtMat;
  }
}

bool IntrinsicCalibrator::runAndSave(const string& outputFilename, bool writeExtrinsics, bool writePoints) {
  vector<Mat> rvecs, tvecs;
  vector<float> reprojErrs;
  double totalAvgErr = 0;

  bool ok = runCalibration(
      rvecs, tvecs, reprojErrs, totalAvgErr);
  printf("%s. avg reprojection error = %.2f\n",
      ok ? "Calibration succeeded" : "Calibration failed",
      totalAvgErr);

  if( ok )
    saveCameraParams(
        outputFilename, 
        writeExtrinsics ? rvecs : vector<Mat>(),
        writeExtrinsics ? tvecs : vector<Mat>(),
        writeExtrinsics ? reprojErrs : vector<float>(),
        totalAvgErr, writePoints);
  return ok;
}
#endif
