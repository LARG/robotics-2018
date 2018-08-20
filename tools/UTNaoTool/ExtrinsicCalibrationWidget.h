#ifndef EXTRINSIC_CALIBRATION_WIDGET_H
#define EXTRINSIC_CALIBRATION_WIDGET_H

#include <iostream>
#include "ui_ExtrinsicCalibrationWidget.h"
#include <memory/LogViewer.h>
#include <common/RobotInfo.h>
#include <vector>
#include <vision/structures/Sample.h>
#include <common/RobotCalibration.h>
#include <calibration/ExtrinsicCalibrator.h>
#include <memory/WorldObjectBlock.h>
#include <common/RobotInfo.h>
#include <common/RobotDimensions.h>
#include "calibration/JointCalibrator.h"

class ExtrinsicCalibrationWidget : public QWidget, public Ui_UTExtrinsicCalibrationWidget {
  Q_OBJECT
  private:
    int currentFrame_;
    Camera::Type currentCamera_;
    ImageProcessor* topProcessor_, *bottomProcessor_;
    LogViewer* log_;
    WorldObjectBlock* world_object_block_;
    vector<Sample> samples_;
    void initializeItems();
    JointCalibrator calibrator_;
  public:
    ExtrinsicCalibrationWidget(QWidget* parent);
    std::string calibration_file_;
    void saveCalibration(std::string);
    void loadCalibration(std::string);
    void loadCalibration(const RobotCalibration& cal, bool includePose = true);
    RobotCalibration getCalibration(bool includePose = true) const;
    void setWorldObjectBlock(WorldObjectBlock* block);
    std::vector<Sample> getSamples() const;
  public slots:
    void handleNewLogFrame(int);
    void setCurrentCamera(Camera::Type);
    void handleNewLogLoaded(LogViewer*);
    void handleNewStreamFrame();
    void setImageProcessors(ImageProcessor* top, ImageProcessor* bottom);

    void resetParameters();
    void clear();
    void save();
    void saveAs();
    void load();
    void takeSamples();
    void optimizeCalibration();
    void stopCalibration();
    void resetCalibration();
    void addSample(Sample);
    void handleUpdatedCalibrations();
  protected slots:
    void toggleList(int state, QWidget* box);
signals:
    void calibrationsUpdated();
};

#endif /* end of include guard: EXTRINSIC_CALIBRATION_WIDGET_H */
