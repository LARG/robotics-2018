#ifndef CALIBRATION_WIDGET_H
#define CALIBRATION_WIDGET_H

#include <QtGui>
#include <QWidget>

#include <map>
#include <vector>
#include <fstream>

#include "ui_IntrinsicCalibrationWidget.h"
#include <common/ColorConversion.h>

#include <memory/LogViewer.h>
#include <vision/VisionModule.h>
#include <math/Point.h>
#ifdef ENABLE_OPENCV
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <calibration/IntrinsicCalibrator.h>
#include <memory/ImageBlock.h>
#endif


class IntrinsicCalibrationWidget : public QWidget, public Ui_UTIntrinsicCalibrationWidget {
  Q_OBJECT

  private:
    bool isCollecting_, paused_;
    int currentFrame_;
    Camera::Type currentCamera_;
    ImageProcessor* topProcessor_, *bottomProcessor_;
    LogViewer* log_;
#ifdef ENABLE_OPENCV
    IntrinsicCalibrator calibrator_;
    void updateProgress(ICProgress p, int count);
#endif
    void clearProgress();

  public:
    IntrinsicCalibrationWidget(QWidget* parent);
    void setImageProcessors(ImageProcessor* top, ImageProcessor* bottom);
    void beginCollection();
    void stopCollection();
  signals:
#ifdef ENABLE_OPENCV
    void calibrationPointsFound(std::vector<cv::Point2f> points);
#endif
  public slots:
    void handleNewLogFrame(int);
    void setCurrentCamera(Camera::Type);
    void handleNewLogLoaded(LogViewer*);
    void handleNewStreamFrame();
    void handleClick(int,int,int);
#ifdef ENABLE_OPENCV
    void saveLogImages();
#endif
    void calibrate();
    void resetCollection();
    void collectClicked();
};

#endif
