#include <VisionWindow.h>

void VisionWindow::changeToRawTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(RAW_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToRawBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(RAW_IMAGE, IMAGE_BOTTOM);
}
void VisionWindow::changeToSegTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(SEG_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToSegBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(SEG_IMAGE, IMAGE_BOTTOM);
}
void VisionWindow::changeToHorizontalBlobTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(HORIZONTAL_BLOB_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToHorizontalBlobBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(HORIZONTAL_BLOB_IMAGE, IMAGE_BOTTOM);
}
void VisionWindow::changeToVerticalBlobTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(VERTICAL_BLOB_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToVerticalBlobBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(VERTICAL_BLOB_IMAGE, IMAGE_BOTTOM);
}
void VisionWindow::changeToObjTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(OBJ_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToObjBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(OBJ_IMAGE, IMAGE_BOTTOM);
}
void VisionWindow::changeToTransformedTop() {
    emit cameraChanged(Camera::TOP);
    changeBigImage(TRANSFORMED_IMAGE, IMAGE_TOP);
}
void VisionWindow::changeToTransformedBottom() {
    emit cameraChanged(Camera::BOTTOM);
    changeBigImage(TRANSFORMED_IMAGE, IMAGE_BOTTOM);
}


void VisionWindow::changeBigImage(int type, int cam) {
  currentBigImageType_ = type;
  currentBigImageCam_ = cam;
  _widgetAssignments[bigImage] = cam;
  bigImage->fill(Qt::black);
  redrawImages();
}

void VisionWindow::calibrationsUpdated() {
    update();
}

void VisionWindow::updateClassificationCheck(bool value) {
  doingClassification_ = value;
  if (doingClassification_ && getImageProcessor(bigImage)->getImg() == NULL){
    std::cout << "No classification without raw images!" << std::endl;
    doingClassification_ = false;
  }
  if (doingClassification_ && !streaming_)
    emit setCore(true);
  redrawImages();
}

void VisionWindow::updateCalibrationCheck(bool value) {
  doingCalibration_ = value;
  ImageProcessor *top = core_->vision_->top_processor_;
  ImageProcessor *bottom = core_->vision_->bottom_processor_;
  top->enableCalibration(value);
  bottom->enableCalibration(value);
  redrawImages();
}

void VisionWindow::updateCheckerboardCheck(bool value) {
  checkerboard_ = value;
  redrawImages();
}

void VisionWindow::updateTable(unsigned char *colorTable, int yIdx, int uIdx, int vIdx) {

  int ySen = classification->yDial->value();
  int uSen = classification->uDial->value();
  int vSen = classification->vDial->value();

  for (int y = max(yIdx - ySen, 0); y <= min(yIdx + ySen, 255); y++) {
    for (int u = max(uIdx - uSen, 0); u <= min(uIdx + uSen, 255); u++) {
      for (int v = max(vIdx - vSen, 0); v <= min(vIdx + vSen, 255); v++) {
          ColorTableMethods::assignColor(colorTable, y,u,v, (Color)classification->colorCombo->currentIndex());
      }
    }
  }

}

void VisionWindow::updateClicked(int xIdx, int yIdx, int buttonIdx){
  if(!initialized_) return;
  int image = currentBigImageCam_;
  ImageProcessor* processor = getImageProcessor(image);
  unsigned char* colorTable = processor->getColorTable();
  const ImageParams& iparams = processor->getImageParams();

  if (doingCalibration_) {
    Sample s; s.x = xIdx; s.y = yIdx;
    if(image == IMAGE_TOP)
      s.camera = Camera::TOP;
    else
      s.camera = Camera::BOTTOM;
    emit calibrationSampleAdded(s);
    redrawImages();
  }

  if (doingClassification_) {
    if (buttonIdx == Qt::LeftButton) {

      //for(int i=0; i < LUT_SIZE; i++)
        //std::cout << colorTable[i] << "\,";
      //std::cout << "DONE\n";
      memcpy(tempTable,colorTable,LUT_SIZE);
      ColorTableMethods::xy2yuv(processor->getImg(), xIdx, yIdx, iparams.width, currentY_, currentU_, currentV_);
      updateTable(colorTable, currentY_, currentU_, currentV_);
      //for(int i=0; i < LUT_SIZE; i++)
        //std::cout << tempTable[i] << "\,";
        //sstd::cout << "\n";
      colorUpdateAvailable_ = true;
      redrawImages();
      processor->processFrame();
      memcpy(colorTable,tempTable,LUT_SIZE);

    } else if (buttonIdx == Qt::RightButton && colorUpdateAvailable_) {

      memcpy(undoTable, colorTable, LUT_SIZE);
      undoImage_ = image;
      updateTable(colorTable, currentY_, currentU_, currentV_);
      colorUpdateAvailable_ = false;

      redrawImages();
    }
  }
}


void VisionWindow::updateToolTip(int image) {
}

