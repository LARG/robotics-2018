#include  "JointCalibrator.h"
#include <memory/MemoryFrame.h>
#include <memory/MemoryCache.h>
#include <memory/LogViewer.h>
#include <vision/ImageProcessor.h>
#include <math/GaussNewtonOptimizer.h>

#define OPTIMIZE_POSITION false

using namespace std;
using namespace Eigen;
using Corners = JointCalibrator::Corners;

std::vector<int> rjointMap = {
  HeadYaw,
  HeadPitch,
  RHipYawPitch,
  RHipRoll,
  RHipPitch,
  RKneePitch,
  RAnklePitch,
  RAnkleRoll
};

std::vector<int> ljointMap = {
  HeadYaw,
  HeadPitch,
  LHipYawPitch,
  LHipRoll,
  LHipPitch,
  LKneePitch,
  LAnklePitch,
  LAnkleRoll
};

std::vector<float> weights = {
  1.0f,
  1.0f,
  1.0f,
  1.0f,
  1.0f,
  1.0f,
  1.0f,
  1.0f,
  100.0f,
  100.0f,
  100.0f,
  1.0f
};

JCSettings::JCSettings() {
  boardOffset = 165.0f;
  squareSize = 25.0f;
  boardSize.width = 6;
  boardSize.height = 4;
  corners = 24;
  oX = oY = 0;
  oZ = -3;
  oT = 0;
}

JointCalibrator::JointCalibrator() {
  memory_ = std::make_unique<MemoryFrame>(false, MemoryOwner::TOOL_MEM, 0, 1);
  //printf("creating cache\n");
  cache_ = std::make_unique<MemoryCache>();
  //printf("creating blocks\n");
  vblocks_ = std::make_unique<VisionBlocks>(*cache_);
  //printf("creating params\n");
  params_ = std::make_unique<ImageParams>(Camera::BOTTOM);
  //printf("creating processor\n");
  processor_ = std::make_unique<ImageProcessor>(*vblocks_, *params_, Camera::BOTTOM);
  processor_->enableCalibration(true);
  datafile_ = string(getenv("NAO_HOME")) + "/data/jcalibration.yaml";
  reset();
}

JointCalibrator::~JointCalibrator() {
  stop();
}

void JointCalibrator::reset() {
  cal_ = std::make_unique<RobotCalibration>();
  processor_->setCalibration(*cal_);
  stop();
}

void JointCalibrator::stop() {
  calibrating_ = false;
  if(thread_) {
    thread_->join();
    thread_.reset();
  }
}

void JointCalibrator::start(int iterations, function<void()> callback) {
  if(thread_) {
    thread_->join();
    thread_.reset();
  }
  calibrating_ = true;
  thread_ = std::make_unique<std::thread>(&JointCalibrator::calibrate, (JointCalibrator*)this, iterations, callback);
}

void JointCalibrator::pause() {
  calibrating_ = false;
  //TODO: fill in
}

void JointCalibrator::setCalibration(const RobotCalibration& cal) {
  cal_ = std::make_unique<RobotCalibration>(cal);
  processor_->setCalibration(*cal_);
  processor_->updateTransform();
}

void JointCalibrator::setMemory(MemoryFrame* memory) {
  //memory_ = std::make_unique<MemoryFrame>(*memory);
  //memory_ = memory;
  cache_->fill(memory);
  processor_->updateTransform();
}

void JointCalibrator::takeSamples(LogViewer* log) {
  printf("getting samples\n");
  Dataset dataset;
  for(int i = 0; i < log->size(); i++) {
    auto& frame = log->getFrame(i);
    cache_->fill(frame);
    auto m = takeSample(&frame);
    printf("found %lu corners in frame %i\n", m.corners.size(), i);
    if(m.corners.size() != settings_.corners) continue;
    dataset.push_back(m);
  }
  dataset.saveToFile(datafile_);
}

JointCalibrator::Measurement JointCalibrator::takeSample(MemoryFrame* frame) {
  unsigned char* imgraw = processor_->getImg();
  const ImageParams& iparams = processor_->getImageParams();
  cv::Mat cvimage = color::rawToMat(imgraw, iparams);
  Measurement m;
  m.left = left();
  m.corners = findChessboardCorners(cvimage);
  if(m.corners.size() == 0) return m;
  MemoryCache cache(frame);
  for(int i = 0; i < jointMap().size(); i++)
    m.joints.push_back(cache.joint->values_[jointMap()[i]]);
  return m;
}

void JointCalibrator::calibrate(int iterations, function<void()> callback) {
  cache_->fill(*memory_);
  Dataset dataset;
  dataset.loadFromFile(datafile_);
  vector<float> iparams = convertParams(*cal_);
  GaussNewtonOptimizer<Measurement,JointCalibrator> optimizer(iparams, dataset, *this, &JointCalibrator::evaluate);
  printf("created optimizer\n");
  for(int i = 0; i < iterations; i++) {
    if(!calibrating_) break;
    error_ = optimizer.iterate();
    printf("ITERATION CHANGE: %2.5f\n", error_);
    auto params = optimizer.getParameters();
    printf("ITERATION ERROR: %2.5f\n", evaluate(dataset, params));
    if(error_ < .01) break;
  }
 printf("\n--------------------------------------\n");
 auto params = optimizer.getParameters();
 *cal_ = convertParams(params);
 callback();
}

vector<float> JointCalibrator::convertParams(const RobotCalibration& cal) const {
  printf("start!\n");
  vector<float> offsets;
  for(int i = 0; i < jointMap().size(); i++) {
    offsets.push_back(cal.jointValues_[jointMap()[i]] / weights[i]);
  }
  printf("poses\n");
  if(OPTIMIZE_POSITION) {
    offsets.push_back(cal.poseX / weights[jointMap().size()]);
    offsets.push_back(cal.poseY / weights[jointMap().size() + 1]);
    offsets.push_back(cal.poseZ / weights[jointMap().size() + 2]);
    offsets.push_back(cal.poseTheta / weights[jointMap().size() + 3]);
  }
  printf("done: %lu\n",offsets.size());
  return offsets;
}


RobotCalibration JointCalibrator::convertParams(const vector<float>& offsets) const {
  RobotCalibration cal;
  cal.useLeft = left();
  //printf("applying offsets:\n");
  for(int i = 0; i < jointMap().size(); i++) {
    //printf("[%s:%2.5f],",JointNames[jointMap()[i]].c_str(),offsets[i]);
    cal.jointValues_[jointMap()[i]] = offsets[i] * weights[i];
  }
  if(OPTIMIZE_POSITION) {
    cal.poseX = offsets[jointMap().size()] * weights[jointMap().size()];
    cal.poseY = offsets[jointMap().size() + 1] * weights[jointMap().size() + 1];
    cal.poseZ = offsets[jointMap().size() + 2] * weights[jointMap().size() + 2];
    cal.poseTheta = offsets[jointMap().size() + 3] * weights[jointMap().size() + 3];
  } else {
    cal.poseX = settings_.oX;
    cal.poseY = settings_.oY;
    cal.poseZ = settings_.oZ;
    cal.poseTheta = settings_.oT;
  }
  //printf("pose offset: %2.3f,%2.3f,%2.3f @ %2.f", cal.poseX, cal.poseY, cal.poseZ, cal.poseTheta * RAD_T_DEG);
  //printf("\n");
  return cal;
}

float JointCalibrator::evaluate(const Dataset& d, const vector<float>& offsets) const {
  float sqerror = 0;
  for(auto m : d) {
    sqerror += pow(evaluate(m,offsets),2);
  }
  return sqrt(sqerror / d.size());
}

float JointCalibrator::evaluate(const Measurement& m, const vector<float>& offsets) const {
  *cal_ = convertParams(offsets);
  cal_->useLeft = m.left; // This is redundant, just want to avoid mistakes
  for(int i = 0; i < jointMap().size(); i++) {
    assert(m.joints.size() == jointMap().size());
    cache_->joint->values_[jointMap()[i]] = m.joints[i];
  }
  processor_->setCalibration(*cal_);
  processor_->updateTransform();
  //printf("projecting corners\n");
  auto pcorners = projectChessboardCorners(m.left);
  //printf("computing projection error\n");
  float error = computeProjectionError(m.corners, pcorners);
  //printf("error: %2.2f\n", error);
  for(int i = 0; i < pcorners.size(); i++) {
    auto mc = m.corners[i];
    auto pc = pcorners[i];
    //printf("[(%2.2f,%2.2f) vs (%2.2f,%2.2f)],", mc[0], mc[1], pc[0], pc[1]);
  }
  //printf("\n");
  return error;
}

Corners JointCalibrator::findChessboardCorners(unsigned char* image) const {
  const ImageParams& iparams = processor_->getImageParams();
  cv::Mat cvimage = color::rawToMat(image, iparams);
  return findChessboardCorners(cvimage);
}

Corners JointCalibrator::findChessboardCorners(cv::Mat& image) const {
  //printf("find corners\n");
  cv::Mat grayImage, sharpened;

  // Sharpen the image first to improve detection rates
  cv::GaussianBlur(image, sharpened, cv::Size(0, 0), 3);
  cv::addWeighted(image, 3, sharpened, -2, 0, sharpened);

  // Convert to gray
  cvtColor(sharpened, grayImage, CV_BGR2GRAY);

  // Detect checkerboard corners
  vector<cv::Point2f> imagePoints;
  bool found = cv::findChessboardCorners(grayImage, settings_.boardSize, imagePoints,
      CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

  // This doesn't seem to be necessary
  //if(settings_.patternType == CalibratorSettings::CHESSBOARD && found) {
    //cv::cornerSubPix(grayImage, imagePoints, cv::Size(11,11), cv::Size(-1, -1),
        //cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
  //}
  auto corners = Corners();
  //printf("%i corners found:\n", imagePoints.size());
  processor_->updateTransform();
  for(auto p : imagePoints) {
    corners.push_back(Vector2f(p.x,p.y));
    //printf("(%2.f,%2.f) ",p.x,p.y);
    const auto& cmat = processor_->getCameraMatrix();
    auto world = cmat.getWorldPosition(p.x,p.y);
    //printf(" <- [%2.f,%2.f,%2.f], ", world.x, world.y, world.z);
  }
  //printf("\n");
  return corners;
}

Corners JointCalibrator::projectChessboardCorners(bool left) const {
  vector<Vector3f> fcorners;
  Vector2f topLeft(
    settings_.boardOffset + (settings_.boardSize.height / 2.0f - .5) * settings_.squareSize,
    (settings_.boardSize.width / 2.0f - .5) * settings_.squareSize
  );
  //printf("top left (foot rel): %2.f,%2.f,0\n",topLeft[0],topLeft[1]);
  for(int x = 0; x < settings_.boardSize.height; x++) {
    for(int y = 0; y < settings_.boardSize.width; y++) {
      fcorners.push_back(Vector3f(
        topLeft[0] - settings_.squareSize * x,
        topLeft[1] - settings_.squareSize * y,
        0
      ));
    }
  }
  Corners corners;
  //printf("converting %i world coordinates\n", fcorners.size());
  for(auto fc : fcorners) {
    Pose3D foot = cache_->body_model->abs_parts_[left ? BodyPart::left_bottom_foot : BodyPart::right_bottom_foot];
    //printf("%s foot at %2.2f,%2.2f,%2.2f\n", left ? "left" : "right", foot.translation.x, foot.translation.y, foot.translation.z);
    foot.translate(cal_->poseX, cal_->poseY, cal_->poseZ);
    foot.rotateZ(cal_->poseTheta);
    foot.translate(fc[0], fc[1], fc[2]);
    auto wc = foot.translation;
    //printf("converting foot-rel to world: %2.2f,%2.2f,%2.2f --> %2.2f,%2.2f,%2.2f\n", fc[0], fc[1], fc[2], wc[0], wc[1], wc[2]);

    const auto& cmat = processor_->getCameraMatrix();
    auto coords = cmat.getImageCoordinates(wc[0], wc[1], wc[2]);
    Vector2f ic(coords.x, coords.y);
    //printf("converting world coordinate: %2.2f,%2.2f,%2.2f --> %2.f,%2.f\n", wc[0], wc[1], wc[2], ic[0], ic[1]);
    corners.push_back(ic);
  }
  return corners;
}

float JointCalibrator::computeProjectionError(const Corners& icorners, const Corners& pcorners) const {
  assert(icorners.size() == pcorners.size());
  float sqSum = 0.0f;
  const ImageParams& iparams = processor_->getImageParams();
  for(int i = 0; i < icorners.size(); i++) {
    if(pcorners[i][0] < 0 || pcorners[i][0] > iparams.width || pcorners[i][1] < 0 || pcorners[i][1] > iparams.height) continue;
    sqSum += (icorners[i] - pcorners[i]).squaredNorm();
  }
  float sum = sqrt(sqSum);
  return sum;
}

bool& JointCalibrator::left() {
  return cal_->useLeft;
}

const bool& JointCalibrator::left() const {
  return cal_->useLeft;
}

vector<int>& JointCalibrator::jointMap() {
  if(left()) return ljointMap;
  else return rjointMap;
}

const vector<int>& JointCalibrator::jointMap() const {
  if(left()) return ljointMap;
  else return rjointMap;
}

bool JointCalibrator::validateProjection(const Corners& icorners, const Corners& pcorners) const {
  return icorners.size() == pcorners.size() && icorners.size() > 0;
}

