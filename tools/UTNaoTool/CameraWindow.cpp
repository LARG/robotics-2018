#include <QtGui>
#include "CameraWindow.h"
#include <iostream>
#include "UTMainWnd.h"

#include <memory/CameraBlock.h>
#include <memory/ImageBlock.h>

using namespace std;

CameraWindow::CameraWindow(QMainWindow* parent)  {

  setupUi(this);
  setWindowTitle(tr("Camera Settings Window"));
  parent_ = parent;

  image_block_ = NULL;
  camera_block_ = NULL;

  topButton->setChecked(false);
  bottomButton->setChecked(true);
  showTop = false;

  connect (topButton, SIGNAL(clicked()), this, SLOT(setTopCamera()));
  connect (bottomButton, SIGNAL(clicked()), this, SLOT(setBottomCamera()));
  connect (sendButton, SIGNAL(clicked()), this, SLOT(sendParams()));
  connect (copyRButton, SIGNAL(clicked()), this, SLOT(copyReadParams()));
  connect (copyWButton, SIGNAL(clicked()), this, SLOT(copySentParams()));
  connect (printButton, SIGNAL(clicked()), this, SLOT(printParams()));
  connect (getButton, SIGNAL(clicked()), this, SLOT(getParams()));

  settingNames[0]="AutoWhiteBalance";
  settingNames[1]="ExposureAuto";
  settingNames[2]="BacklightCompensation";

  settingNames[3]="Brightness";
  settingNames[4]="Contrast";
  settingNames[5]="Saturation";
  settingNames[6]="Hue";
  settingNames[7]="Exposure";
  settingNames[8]="Gain";
  settingNames[9]="Sharpness";

  settingNotes[0]="(disabled)";
  settingNotes[1]="(disabled)";
  settingNotes[2]="(disabled)";

  settingNotes[3]="(Does not work)";
  settingNotes[4]="(Works in 20-53 range)";
  settingNotes[5]="(around 128)";
  settingNotes[6]="(Does not work)";
  settingNotes[7]="(disabled)";
  settingNotes[8]="(disabled)";
  settingNotes[9]="(Effect Unknown)";

  QLabel* settingLabel = new QLabel("Setting");
  QLabel* settingTValue = new QLabel("Top Sent");
  QLabel* settingBValue = new QLabel("Bottom Sent");
  QLabel* readTValue = new QLabel("Top Read");
  QLabel* readBValue = new QLabel("Bottom Read");
  QLabel* setValue = new QLabel("Set");

  settingLabel->setFont( QFont( "Arial", 10, QFont::Bold ) );
  settingTValue->setFont( QFont( "Arial", 10, QFont::Bold ) );
  settingBValue->setFont( QFont( "Arial", 10, QFont::Bold ) );
  readTValue->setFont( QFont( "Arial", 10, QFont::Bold ) );
  readBValue->setFont( QFont( "Arial", 10, QFont::Bold ) );
  setValue->setFont( QFont( "Arial", 10, QFont::Bold ) );

  labelLayout->addWidget(settingLabel,0,0);
  labelLayout->addWidget(readTValue,0,1);
  labelLayout->addWidget(settingTValue,0,2);
  labelLayout->addWidget(readBValue,0,3);
  labelLayout->addWidget(settingBValue,0,4);
  labelLayout->addWidget(setValue,0,5);

  settingLabels = new QLabel[NUM_CAM_SETTINGS];
  settingTValues = new QLabel[NUM_CAM_SETTINGS];
  settingBValues = new QLabel[NUM_CAM_SETTINGS];
  readTValues = new QLabel[NUM_CAM_SETTINGS];
  readBValues = new QLabel[NUM_CAM_SETTINGS];
  controlSpins = new QSpinBox[NUM_CAM_SETTINGS];

  // set labels
  for (int i = 0; i < NUM_CAM_SETTINGS; i++) {
    settingLabels[i].setText(settingNames[i] + " " + settingNotes[i]);
    settingTValues[i].setText(QString::number(0.0));
    settingBValues[i].setText(QString::number(0.0));
    readTValues[i].setText(QString::number(0.0));
    readBValues[i].setText(QString::number(0.0));
    controlSpins[i].setValue(0);

    // add to layout
    labelLayout->addWidget(&settingLabels[i], i+1, 0);
    labelLayout->addWidget(&readTValues[i], i+1, 1);
    labelLayout->addWidget(&settingTValues[i], i+1, 2);
    labelLayout->addWidget(&readBValues[i], i+1, 3);
    labelLayout->addWidget(&settingBValues[i], i+1, 4);
    labelLayout->addWidget(&controlSpins[i], i+1, 5);

  }

  // set ranges for control settings
  controlSpins[0].setRange(0,1);
  controlSpins[1].setRange(0,1);
  controlSpins[2].setRange(0,4);

  controlSpins[3].setRange(0,255);
  controlSpins[4].setRange(0,127);
  controlSpins[5].setRange(0,255);
  controlSpins[6].setRange(-180,180);
  controlSpins[7].setRange(0,512);
  controlSpins[8].setRange(0,255);
  controlSpins[9].setRange(-7,7);

  setWindowTitle(tr("Camera Settings"));
}

void CameraWindow::update(MemoryFrame* memory) {

  memory->getBlockByName(image_block_, "raw_image",false);
  memory->getBlockByName(camera_block_, "camera_info", false);

  drawRawImage(bigImage);
  bigImage->update();

  if (showTop)
    cameraLabel->setText("Top Cam");
  else
    cameraLabel->setText("Bottom Cam");

  // get params for both top and bottom cameras
  if (camera_block_) {
    setParamLabels(&(camera_block_->params_top_camera_), settingTValues);
    setParamLabels(&(camera_block_->params_bottom_camera_), settingBValues);
    setParamLabels(&(camera_block_->read_params_top_camera_), readTValues);
    setParamLabels(&(camera_block_->read_params_bottom_camera_), readBValues);
  }

}

void CameraWindow::drawRawImage(ImageWidget *image) {

  if (image_block_ == NULL || image_block_->getImgTop() == nullptr || image_block_->getImgBottom() == nullptr) {
    image->fill(0);
    return;
  }

  const unsigned char* imgPtr = NULL;
  ImageParams* iparams;
  if (showTop) {
    imgPtr = image_block_->getImgTop();
    iparams = &image_block_->top_params_;
  }
  else {
    imgPtr = image_block_->getImgBottom();
    iparams = &image_block_->bottom_params_;
  }

  if (imgPtr == NULL) return;

  for (int y = 0; y < iparams->height; y++) {
    for (int x = 0; x < iparams->width; x+=2) {

      color::Yuv422 yuyv;
      yuyv.y0 = (int) (*(imgPtr++));
      yuyv.u = (int) (*(imgPtr++));
      yuyv.y1 = (int) (*(imgPtr++));
      yuyv.v = (int) (*(imgPtr++));

      color::Rgb rgb1, rgb2;
      color::yuv422ToRgb(yuyv, rgb1, rgb2);

      // First pixel
      QRgb value1 = qRgb(rgb1.r, rgb1.g, rgb1.b);
      image->setPixel(x, y, value1);

      // Second Pixel
      QRgb value2 = qRgb(rgb2.r, rgb2.g, rgb2.b);
      image->setPixel(x + 1, y, value2);

    }
  }

}

void CameraWindow::setParamLabels(CameraParams* params, QLabel* values) {

  values[0].setText(QString::number(params->kCameraAutoWhiteBalance));
  values[1].setText(QString::number(params->kCameraExposureAuto));
  values[2].setText(QString::number(params->kCameraBacklightCompensation));

  values[3].setText(QString::number(params->kCameraBrightness));
  values[4].setText(QString::number(params->kCameraContrast));
  values[5].setText(QString::number(params->kCameraSaturation));
  values[6].setText(QString::number(params->kCameraHue));
  values[7].setText(QString::number(params->kCameraExposure));
  values[8].setText(QString::number(params->kCameraGain));
  values[9].setText(QString::number(params->kCameraSharpness));
}

void CameraWindow::setTopCamera() {
  showTop = true;
}

void CameraWindow::setBottomCamera() {
  showTop = false;
}

void CameraWindow::setControlParams(CameraParams* params){

  controlSpins[0].setValue(params->kCameraAutoWhiteBalance);
  controlSpins[1].setValue(params->kCameraExposureAuto);
  controlSpins[2].setValue(params->kCameraBacklightCompensation);

  controlSpins[3].setValue(params->kCameraBrightness);
  controlSpins[4].setValue(params->kCameraContrast);
  controlSpins[5].setValue(params->kCameraSaturation);
  controlSpins[6].setValue(params->kCameraHue);
  controlSpins[7].setValue(params->kCameraExposure);
  controlSpins[8].setValue(params->kCameraGain);
  controlSpins[9].setValue(params->kCameraSharpness);

}

void CameraWindow::sendParams() {
  ToolPacket tp(ToolPacket::SetBottomCameraParameters);
  if (showTop)
    tp.message = ToolPacket::SetTopCameraParameters;
  
  QString cmd;
  for (int i = 0; i < NUM_CAM_SETTINGS; i++) {
    cmd += settingNames[i] + " "
         + QString::number(controlSpins[i].value()) + " ";
  }
  cmd += "|";
  snprintf(tp.data.data(), ToolPacket::DATA_LENGTH, cmd.toStdString().c_str());
  UTMainWnd::inst()->sendUDPCommandToCurrent(tp);
}


void CameraWindow::printParams() {

  cout << endl << endl << "For cfgcam.lua:" << endl << endl;

  QString settings;

  if (showTop) {
    settings = QString("cfgCamTop");
  } else {
    settings = QString("cfgCamBottom");
  }

  cout << settings.toStdString() << " = {}" << endl;
  cout << settings.toStdString() << ".kCameraAutoWhiteBalance = " << controlSpins[0].value() << endl;
  cout << settings.toStdString() << ".kCameraExposureAuto = " << controlSpins[1].value() << endl;
  cout << settings.toStdString() << ".kCameraBacklightCompensation = " << controlSpins[2].value() << endl;
  cout << settings.toStdString() << ".kCameraBrightness = " << controlSpins[3].value() << endl;
  cout << settings.toStdString() << ".kCameraContrast = " << controlSpins[4].value() << endl;
  cout << settings.toStdString() << ".kCameraSaturation = " << controlSpins[5].value() << endl;
  cout << settings.toStdString() << ".kCameraHue = " << controlSpins[6].value() << endl;
  cout << settings.toStdString() << ".kCameraExposure = " << controlSpins[7].value() << endl;
  cout << settings.toStdString() << ".kCameraGain = " << controlSpins[8].value() << endl;
  cout << settings.toStdString() << ".kCameraSharpness = " << controlSpins[9].value() << endl;

  cout << endl << endl;

}

void CameraWindow::copyReadParams() {
  copyParams(true);
}

void CameraWindow::copySentParams(){
  copyParams(false);
}

void CameraWindow::copyParams(bool readParams){
  if (showTop) {
    if (readParams)
      setControlParams(&(camera_block_->read_params_top_camera_));
    else
      setControlParams(&(camera_block_->params_top_camera_));
  } else {
    if (readParams)
      setControlParams(&(camera_block_->read_params_bottom_camera_));
    else
      setControlParams(&(camera_block_->params_bottom_camera_));
  }
}


void CameraWindow::getParams() {
  ToolPacket tp(ToolPacket::GetCameraParameters);
  UTMainWnd::inst()->sendUDPCommandToCurrent(tp);
}
