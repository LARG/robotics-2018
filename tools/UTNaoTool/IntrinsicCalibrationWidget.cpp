#include "IntrinsicCalibrationWidget.h"
#include <iostream>

using namespace std;

IntrinsicCalibrationWidget::IntrinsicCalibrationWidget(QWidget* parent) : QWidget(parent), log_(0) {
  setupUi(this);

#ifdef ENABLE_OPENCV
  lblOpenCV->setHidden(true);
#else
  grpProgress->setEnabled(false);
  btnCollect->setEnabled(false);
  btnReset->setEnabled(false);
  btnSave->setEnabled(false);
  btnCalibrate->setEnabled(false);
#endif

  currentCamera_ = Camera::TOP;
  isCollecting_ = false;
  paused_ = false;
#ifdef ENABLE_OPENCV
  connect(btnSave, SIGNAL(clicked()), this, SLOT(saveLogImages()));
#endif
  connect(btnCollect, SIGNAL(clicked()), this, SLOT(collectClicked()));
  connect(btnReset, SIGNAL(clicked()), this, SLOT(resetCollection()));
  connect(btnCalibrate, SIGNAL(clicked()), this, SLOT(calibrate()));
}

void IntrinsicCalibrationWidget::handleNewLogFrame(int frame){
  currentFrame_ = frame;
}

void IntrinsicCalibrationWidget::handleNewStreamFrame() {
  if(!isCollecting_) return;
  if(paused_) return;
  ImageProcessor* processor;
  
  if(currentCamera_ == Camera::TOP)
    processor = topProcessor_;
  else 
    processor = bottomProcessor_;
  
#ifdef ENABLE_OPENCV
  unsigned char* imgraw = processor->getImg();
  const ImageParams& iparams = processor->getImageParams();
  cv::Mat cvimage = color::rawToMat(imgraw, iparams);
  vector<cv::Point2f> points = calibrator_.addImage(cvimage);
  ICProgress p = calibrator_.getProgress();
  int count = calibrator_.getSampleCount();
  emit calibrationPointsFound(points);
  updateProgress(p, count);
#endif
}

void IntrinsicCalibrationWidget::clearProgress() {
  progressX->setValue(0);
  progressY->setValue(0);
  progressSize->setValue(0);
  progressSkew->setValue(0);
  lblCount->setText("0 / 40");
}

#ifdef ENABLE_OPENCV
void IntrinsicCalibrationWidget::updateProgress(ICProgress p, int count) {
  progressX->setValue(p.x * 100);
  progressY->setValue(p.y * 100);
  progressSize->setValue(p.size * 100);
  progressSkew->setValue(p.skew * 100);
  lblCount->setText(QString::number(count) + " / 40");
}
#endif

void IntrinsicCalibrationWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom){
  topProcessor_ = top;
  bottomProcessor_ = bottom;
}

void IntrinsicCalibrationWidget::setCurrentCamera(Camera::Type camera){
  currentCamera_ = camera;
}

void IntrinsicCalibrationWidget::handleNewLogLoaded(LogViewer* log){
  log_ = log;
}

void IntrinsicCalibrationWidget::handleClick(int x, int y, int button) {
  button = button; // kill warning
  x = x; y = y;
}

void IntrinsicCalibrationWidget::collectClicked() {
  if(isCollecting_) {
    if(paused_)
      beginCollection();
    else
      stopCollection();
  } else beginCollection();
}

void IntrinsicCalibrationWidget::beginCollection() {
  if(!isCollecting_) {
#ifdef ENABLE_OPENCV
    calibrator_ = IntrinsicCalibrator();
#endif
    isCollecting_ = true;
  }
  paused_ = false;
  btnCollect->setText("Pause Collection");
}

void IntrinsicCalibrationWidget::stopCollection() {
  paused_ = true;
  btnCollect->setText("Collect Samples");
}

void IntrinsicCalibrationWidget::resetCollection() {
  isCollecting_ = false;
  clearProgress();
}

void IntrinsicCalibrationWidget::calibrate() {
#ifdef ENABLE_OPENCV
  calibrator_.runAndSave("calibration.yaml");
#endif
}

#ifdef ENABLE_OPENCV
void IntrinsicCalibrationWidget::saveLogImages() {
  if(!log_) return;
  QFileDialog dialog;
  dialog.setOptions(QFileDialog::ShowDirsOnly);
  dialog.setOptions(QFileDialog::DontUseNativeDialog);
  dialog.setFileMode(QFileDialog::Directory);
  QString directory = dialog.getExistingDirectory(this, tr("Save Log Images"), QString(getenv("NAO_HOME")) + "/logs/images");
  cv::Mat mat;
  vector<ImageBuffer> images = log_->getRawTopImages();
  vector<ImageParams> params = log_->getTopParams();
  for(size_t i = 0; i < images.size(); i++) {
    ImageParams& iparams = params[i];
    cv::Mat image = color::rawToMat(images[i], iparams);
    char buf[100];
    sprintf(buf,"%03i",i);
    QString file = directory + "/log_image_" + buf + ".png";
    cv::imwrite(file.toStdString(), image);
  }
}
#endif
