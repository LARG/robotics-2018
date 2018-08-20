#ifndef CAMERA_WINDOW_H
#define CAMERA_WINDOW_H

#include <QWidget>
#include <QSpinBox>

#include "ImageWidget.h"
#include <common/RobotInfo.h>
#include <memory/MemoryFrame.h>

#include <common/ColorConversion.h>
#include <common/CameraParams.h>

#include "ui_CameraWindow.h"
#define NUM_CAM_SETTINGS 10

class ImageBlock;
class CameraBlock;

class CameraWindow : public QMainWindow, public Ui_UTCameraWindow {
 Q_OBJECT

  public:
    CameraWindow(QMainWindow* parent);

    void update(MemoryFrame *memory);
    void drawRawImage(ImageWidget *image);
    void setParamLabels(CameraParams* params, QLabel* values);
    void setControlParams(CameraParams* params);

    QLabel* settingLabels;
    QLabel* readTValues;
    QLabel* readBValues;
    QLabel* settingTValues;
    QLabel* settingBValues;
    QSpinBox* controlSpins;

    QString settingNames[NUM_CAM_SETTINGS];
    QString settingNotes[NUM_CAM_SETTINGS];

    QMainWindow* parent_;

    ImageBlock *image_block_;
    CameraBlock *camera_block_;

    bool showTop;

  public slots:

    void setTopCamera();
    void setBottomCamera();

    void sendParams();
    void printParams();
    void getParams();
    void copyParams(bool readParams);
    void copyReadParams();
    void copySentParams();

};

#endif
