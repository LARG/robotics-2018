#include <QtGui>
#include <algorithm>

#include <tool/VisionWindow.h>
#include <tool/UTMainWnd.h>
#include <common/ColorConversion.h>

#include <memory/RobotVisionBlock.h>
#include <memory/ImageBlock.h>
#include <memory/CameraBlock.h>
#include <memory/JointBlock.h>
#include <memory/SensorBlock.h>
#include <memory/WorldObjectBlock.h>
#include <memory/BodyModelBlock.h>
#include <memory/RobotStateBlock.h>

#include <common/WorldObject.h>
#include <common/Field.h>
#include <common/ColorSpaces.h>

#include <vision/ImageProcessor.h>
#include <common/annotations/SelectionType.h>
#include <tool/calibration/JointCalibrator.h>

VisionWindow::VisionWindow(QMainWindow* parent, VisionCore *core) :
    ConfigWindow(parent),
    core_(core),
    parent_(parent),
    currentBigImageType_(RAW_IMAGE),
    currentBigImageCam_(Camera::TOP),
    robot_vision_block_(nullptr),
    image_block_(nullptr),
    camera_block_(nullptr),
    joint_block_(nullptr),
    sensor_block_(nullptr),
    world_object_block_(nullptr),
    body_model_block_(nullptr),
    robot_state_block_(nullptr),
    last_memory_(nullptr),
    vision_memory_(new MemoryFrame(false,MemoryOwner::TOOL_MEM, 0, 1)),
    initialized_(false),
    enableDraw_(true)
  {

  setupUi(this);
  setWindowTitle(tr("Vision Window"));
  streaming_ = false;

  qApp->installEventFilter(this);

  bigImage->setMouseTracking(true);

  // Connect UI objects
  connect(rawImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToRawTop()));  // For some reason it didn't like using
  connect(rawImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToRawBottom()));  // For some reason it didn't like using
  connect(segImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToSegTop()));  // the #defines RAWIMAGE etc
  connect(segImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToSegBottom()));  // the #defines RAWIMAGE etc
  connect(horizontalBlobImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToHorizontalBlobTop()));
  connect(horizontalBlobImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToHorizontalBlobBottom()));
  connect(verticalBlobImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToVerticalBlobTop()));
  connect(verticalBlobImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToVerticalBlobBottom()));
  connect(objImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToObjTop()));
  connect(objImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToObjBottom()));
  connect(transformedImageTop, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToTransformedTop()));
  connect(transformedImageBottom, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(changeToTransformedBottom()));

  connect(bigImage, SIGNAL(moved(int,int)), this, SLOT(updateCursorInfo(int,int)));
  connect(bigImage, SIGNAL(clicked(int,int,Qt::MouseButton)), this, SLOT(handleClicked(int,int,Qt::MouseButton)));

  connect(classification->classificationBox, SIGNAL(toggled(bool)), this, SLOT(updateClassificationCheck(bool)));
  connect(cbxCalibration, SIGNAL(toggled(bool)), this, SLOT(updateCalibrationCheck(bool)));
  connect(cbxOverlay, SIGNAL(clicked()), this, SLOT(controlsChanged()));
  connect(cbxHorizon, SIGNAL(clicked()), this, SLOT(controlsChanged()));
  connect(cbxTooltip, SIGNAL(clicked()), this, SLOT(controlsChanged()));
  connect(cbxCalibration, SIGNAL(clicked()), this, SLOT(controlsChanged()));
  connect(cbxCheckerboard, SIGNAL(clicked()), this, SLOT(controlsChanged()));

  connect(actionNew_Bottom, SIGNAL(triggered()), this, SLOT(bottomNewTable()) );
  connect(actionOpen_Bottom, SIGNAL(triggered()), this, SLOT(bottomOpenTable()) );
  connect(actionSave_As_Bottom, SIGNAL(triggered()), this, SLOT(bottomSaveTableAs()) );
  connect(actionSave_Bottom, SIGNAL(triggered()), this, SLOT(bottomSaveTable()) );

  connect(actionNew_Top, SIGNAL(triggered()), this, SLOT(topNewTable()) );
  connect(actionOpen_Top, SIGNAL(triggered()), this, SLOT(topOpenTable()) );
  connect(actionSave_As_Top, SIGNAL(triggered()), this, SLOT(topSaveTableAs()) );
  connect(actionSave_Top, SIGNAL(triggered()), this, SLOT(topSaveTable()) );
  

  connect(bigImage, SIGNAL(selected(Selection*)), annot, SLOT(selected(Selection*)));
  connect(bigImage, SIGNAL(dragged(int,int,Qt::MouseButton)), annot, SLOT(handleDragged(int,int,Qt::MouseButton)));
  connect(bigImage, SIGNAL(hovered(int,int)), annot, SLOT(handleHovered(int,int)));
  connect(annot, SIGNAL(selectionTypeChanged(SelectionType)), bigImage, SLOT(selectionTypeChanged(SelectionType)));
  connect(annot, SIGNAL(selectionEnabled(bool)), bigImage, SLOT(setSelectionEnabled(bool)));
  connect(annot, SIGNAL(setCurrentSelections(std::vector<Selection*>)), bigImage, SLOT(setCurrentSelections(std::vector<Selection*>)));
  connect(annot, SIGNAL(setCurrentLogFrame(int)), UTMainWnd::inst(), SLOT(gotoSnapshot(int)));
  connect(this, SIGNAL(newLogLoaded(LogViewer*)), annot, SLOT(handleNewLogLoaded(LogViewer*)));
  connect(this, SIGNAL(newLogFrame(int)), annot, SLOT(handleNewLogFrame(int)));
  connect(this, SIGNAL(cameraChanged(Camera::Type)), annot, SLOT(setCurrentCamera(Camera::Type)));

  connect(this, SIGNAL(cameraChanged(Camera::Type)), classification, SLOT(setCurrentCamera(Camera::Type)));
  connect(this, SIGNAL(newLogLoaded(LogViewer*)), classification, SLOT(handleNewLogLoaded(LogViewer*)));
  connect(annot, SIGNAL(setCurrentAnnotations(std::vector<VisionAnnotation*>)), classification, SLOT(setAnnotations(std::vector<VisionAnnotation*>)));
  connect(classification, SIGNAL(colorTableGenerated()), this, SLOT(update()));
  connect(classification->undoButton, SIGNAL(clicked()), this, SLOT(doUndo()));

  connect(this, SIGNAL(cameraChanged(Camera::Type)), analysis, SLOT(setCurrentCamera(Camera::Type)));
  connect(this, SIGNAL(newLogLoaded(LogViewer*)), analysis, SLOT(handleNewLogLoaded(LogViewer*)));
  connect(annot, SIGNAL(setCurrentAnnotations(std::vector<VisionAnnotation*>)), analysis, SLOT(setAnnotations(std::vector<VisionAnnotation*>)));
  connect(analysis, SIGNAL(colorTableGenerated()), this, SLOT(update()));
  connect(classification, SIGNAL(colorTableGenerated()), analysis, SLOT(handleColorTableGenerated()));
  connect(this, SIGNAL(colorTableLoaded()), analysis, SLOT(handleColorTableGenerated()));
  connect(this, SIGNAL(newLogFrame(int)), analysis, SLOT(handleNewLogFrame(int)));
  connect(analysis, SIGNAL(memoryChanged()), this, SLOT(update()));

  connect(this, SIGNAL(newLogFrame(int)), icalibration, SLOT(handleNewLogFrame(int)));
  connect(this, SIGNAL(newLogLoaded(LogViewer*)), icalibration, SLOT(handleNewLogLoaded(LogViewer*)));
  connect(this, SIGNAL(newStreamFrame()), icalibration, SLOT(handleNewStreamFrame()));
  
  connect(this, SIGNAL(newLogFrame(int)), ecalibration, SLOT(handleNewLogFrame(int)));
  connect(this, SIGNAL(newLogLoaded(LogViewer*)), ecalibration, SLOT(handleNewLogLoaded(LogViewer*)));
  connect(this, SIGNAL(newStreamFrame()), ecalibration, SLOT(handleNewStreamFrame()));
  
  connect(this, SIGNAL(calibrationSampleAdded(Sample)), ecalibration, SLOT(addSample(Sample)));
  connect(ecalibration->clearButton, SIGNAL(clicked()), this, SLOT(clearSamples()));
  connect(ecalibration, SIGNAL(calibrationsUpdated()), this, SLOT(calibrationsUpdated()));

  connect(this, SIGNAL(colorTableLoaded()), this, SLOT(update()));

  connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(controlsChanged(int)));

  bigImage->setSelectionEnabled(false);

  // Set rgb for the segmented colors for drawing
  segRGB[c_UNDEFINED] = qRgb(0, 0, 0);
  segRGB[c_FIELD_GREEN] = qRgb(0, 150, 0);
  segRGB[c_WHITE] = qRgb(255, 255, 255);
  segRGB[c_ORANGE] = qRgb(255, 125, 0);
  segRGB[c_PINK] = qRgb(255, 64, 64);
  segRGB[c_BLUE] = qRgb(128, 128, 192);
  segRGB[c_YELLOW] = qRgb(255, 255, 0);
  segRGB[c_ROBOT_WHITE] = qRgb(185, 185, 185);

  for (int i=0; i<Color::NUM_Colors; i++) {
    segCol[i].setRgb(segRGB[i]);
  }
  sampleColor.setRgb(qRgb(255, 0, 0));
  calibrationLineColor.setRgb(qRgb(0,255,239));
  connectionLineColor.setRgb(qRgb(255,0,0));

  doingClassification_ = false;

  doingCalibration_ = false;
  jcalibrator_ = new JointCalibrator();
  assignProcessors();
  assignImageWidgets();
  timer_.start();
  emit cameraChanged(Camera::TOP);
}

void VisionWindow::assignImageWidgets() {
  _widgetAssignments[rawImageTop] =
    _widgetAssignments[segImageTop] =
    _widgetAssignments[horizontalBlobImageTop] =
    _widgetAssignments[verticalBlobImageTop] =
    _widgetAssignments[objImageTop] =
    _widgetAssignments[transformedImageTop] = Camera::TOP;
  _widgetAssignments[rawImageBottom] =
    _widgetAssignments[segImageBottom] =
    _widgetAssignments[horizontalBlobImageBottom] =
    _widgetAssignments[verticalBlobImageBottom] =
    _widgetAssignments[objImageBottom] =
    _widgetAssignments[transformedImageBottom] = Camera::BOTTOM;

  _widgetAssignments[bigImage] = Camera::TOP;
}

void VisionWindow::assignProcessors() {
  ImageProcessor *top = core_->vision_->top_processor(), *bottom = core_->vision_->bottom_processor();
  _imageProcessors[Camera::TOP] = top;
  _imageProcessors[Camera::BOTTOM] = bottom;

  annot->setImageProcessors(top,bottom);
  icalibration->setImageProcessors(top,bottom);
  ecalibration->setImageProcessors(top,bottom);
  classification->setImageProcessors(top,bottom);
  analysis->setImageProcessors(top,bottom);
}

void VisionWindow::setImageSizes() {
  for(std::map<ImageWidget*,int>::iterator it = _widgetAssignments.begin(); it != _widgetAssignments.end(); it++) {
    ImageWidget* widget = it->first;
    ImageProcessor* processor = _imageProcessors[it->second];
    const ImageParams& iparams = processor->getImageParams();
    widget->setImageSize(iparams.width, iparams.height);
  }
}

VisionWindow::~VisionWindow() {
}

void VisionWindow::clearSamples(){
  redrawImages();
}

void VisionWindow::update(){
  analysis->setCore(core_);
  if(last_memory_ == NULL)
    return;
  update(last_memory_);
}

void VisionWindow::handleRunningCore() {
  enableDraw_ = false;
  assignProcessors();
  enableDraw_ = true;
}

void VisionWindow::update(MemoryFrame* memory) {
  initialized_ = true;
  last_memory_ = memory;
  *vision_memory_ = *memory;
  core_->updateMemory(vision_memory_);

  vision_memory_->getBlockByName(robot_vision_block_, "robot_vision");
  vision_memory_->getBlockByName(image_block_, "raw_image");
  vision_memory_->getBlockByName(camera_block_, "camera_info");
  vision_memory_->getBlockByName(joint_block_, "vision_joint_angles");
  vision_memory_->getBlockByName(sensor_block_, "vision_sensors");
  vision_memory_->getBlockByName(world_object_block_, "world_objects");
  vision_memory_->getBlockByName(body_model_block_, "vision_body_model");
  vision_memory_->getBlockByName(robot_state_block_, "robot_state");

  ImageProcessor *top = core_->vision_->top_processor();
  ImageProcessor *bottom = core_->vision_->bottom_processor();
  RobotCalibration cal = ecalibration->getCalibration();
  top->setCalibration(cal);
  bottom->setCalibration(cal);

  // run core to get intermediate vision debug
  if(UTMainWnd::inst()->isRunningCore() || doingClassification_) {
    core_->vision_->processFrame();
  } else {
    core_->vision_->updateTransforms();
  }
  redrawImages();
}

void VisionWindow::updateCursorInfo(int x, int y) {
  if(!initialized_) return;
  int image = currentBigImageCam_;
  ImageProcessor* processor = getImageProcessor(image);
  const ImageParams& iparams = processor->getImageParams();

  unsigned char *segImage = processor->getSegImg(), *rawImage = processor->getImg();
  ImageWidget* rawWidget;

  switch(image) {
    case Camera::TOP:
      rawWidget = rawImageTop;
      break;
    case Camera::BOTTOM:
      rawWidget = rawImageBottom;
      break;
    default: return;
  }

  if (x < 0 || x >= iparams.width || y < 0 || y >= iparams.height) {
    xyLabel->setText("XY ");
    rgbLabel->setText("RGB ");
    yuvLabel->setText("YUV ");
    segLabel->setText("Seg ");
    mouseOverBlobIndex_ = -1;
    mouseOverLineIndex_ = -1;
    mouseOverObjectIndex_ = -1;
    updateToolTip(image);
    return;
  }

  xyLabel->setText("XY ("+QString::number(x)+","+QString::number(y)+")");
  QColor color(rawWidget->getPixel(x,y));
  rgbLabel->setText("RGB ("+QString::number(color.red())+","+QString::number(color.green())+","+QString::number(color.blue())+")");

  // get right index of yuyv
  int yVal, uVal, vVal;
  if (rawImage != NULL){
    ColorTableMethods::xy2yuv(rawImage,x,y,iparams.width, yVal,uVal,vVal);
    yuvLabel->setText("YUV ("+QString::number(yVal)+","+QString::number(uVal)+","+QString::number(vVal)+")");
  } else {
    yuvLabel->setText("YUV (N/A)");
  }

  // get seg color
  if (robot_vision_block_ != NULL && segImage != NULL) {
    int c = segImage[iparams.width * y + x];
    segLabel->setText("Seg ("+QString::number(c)+")");
  } else {
    segLabel->setText("Seg (N/A)");
  }

  if (!core_ || !core_->vision_ || !config_.tooltip) {
    mouseOverBlobIndex_ = -1;
    mouseOverLineIndex_ = -1;
    mouseOverObjectIndex_ = -1;
    updateToolTip(image);
    return;
  }

  // see if we're over a blob or a line
  mouseOverBlobIndex_ = -1;
  mouseOverBlobType_ = -1;

  if (currentBigImageType_ == VERTICAL_BLOB_IMAGE && mouseOverBlobType_ == -1) {
    updateCursorInfoVertical(x,y,image);
  }

  if (currentBigImageType_ == HORIZONTAL_BLOB_IMAGE && mouseOverBlobType_ == -1) {
    updateCursorInfoHorizontal(x,y,image);
  }

  // see if we're over a line
  mouseOverLineIndex_ = -1;
  mouseOverLineType_ = 0;
  if (currentBigImageType_ == RAW_IMAGE && config_.all) {
    updateCursorInfoRaw(x,y,image);
  }

  // see if we're over an object
  mouseOverObjectIndex_ = -1;
  if (currentBigImageType_ == OBJ_IMAGE && world_object_block_ != NULL) {
    updateCursorInfoObj(x,y,image);
  }

  updateToolTip(image);
}

void VisionWindow::updateCursorInfoVertical(int x, int y, int image) {
}

void VisionWindow::updateCursorInfoHorizontal(int x, int y, int image) {
}

void VisionWindow::updateCursorInfoRaw(int x, int y, int image){
}

void VisionWindow::updateCursorInfoObj(int x, int y, int image) {
}

void VisionWindow::saveImages() {

  static int imageNum = 0;

  char rawImageName[128];
  char segImageName[128];
  char horizontalBlobImageName[128];
  char verticalBlobImageName[128];
  char objImageName[128];
  char transformedImageName[128];

  sprintf(rawImageName, "./images/raw-%04d.ppm", imageNum);
  sprintf(segImageName, "./images/seg-%04d.ppm", imageNum);
  sprintf(horizontalBlobImageName, "./images/horzBlob%04d.ppm", imageNum);
  sprintf(verticalBlobImageName, "./images/vertBlob-%04d.ppm", imageNum);
  sprintf(objImageName, "./images/obj-%04d.ppm", imageNum);
  sprintf(transformedImageName, "./images/transformed%04d.ppm", imageNum++);

  rawImageTop->save(QString(rawImageName), "PPM", -1);
  segImageTop->save(QString(segImageName), "PPM", -1);
  horizontalBlobImageTop->save(QString(horizontalBlobImageName), "PPM", -1);
  verticalBlobImageTop->save(QString(verticalBlobImageName), "PPM", -1);
  objImageTop->save(QString(objImageName), "PPM", -1);
  transformedImageTop->save(QString(transformedImageName), "PPM", -1);

  std::cout <<"Images saved..." << std::endl;

}

void VisionWindow::doUndo() {
  ImageProcessor* processor = getImageProcessor(undoImage_);
  unsigned char* colorTable = processor->getColorTable();
  memcpy(colorTable, undoTable, LUT_SIZE);
  redrawImages();
}

ImageProcessor* VisionWindow::getImageProcessor(ImageWidget* widget){
  int image = getImageAssignment(widget);
  return getImageProcessor(image);
}

ImageProcessor* VisionWindow::getImageProcessor(int image){
  return _imageProcessors[image];
}

int VisionWindow::getImageAssignment(ImageWidget* widget){
  return _widgetAssignments[widget];
}

void VisionWindow::handleNewLogFrame(int frame){
  assignProcessors();
  frame_ = frame;
  if(!this->isVisible()) return;
  emit newLogFrame(frame_);
}

void VisionWindow::handleNewStreamFrame() {
  emit newStreamFrame();
}

void VisionWindow::handleNewLogLoaded(LogViewer* log){
  assignProcessors();
  log_ = log;
  frame_ = -1;
  if(!this->isVisible()) return;
  emit newLogLoaded(log_);
}

bool VisionWindow::eventFilter(QObject *object, QEvent *e) {
  if (e->type() == QEvent::KeyPress)
  {
    // We have to do a name check because QWidgets like to swallow or resend events.
    // The annotationList and UTVisionWindow don't seem to ever send the same event,
    // so this should work.
    std::string oName = object->objectName().toStdString();
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
    static const auto validNames = std::set<std::string>({"annotationList", "UTVisionWindow", "lstSources"});
    if(validNames.find(oName) == validNames.end()) return false;
    e->setAccepted(false);
    int mods = QApplication::keyboardModifiers();
    switch (keyEvent->key()) {
      case Qt::Key_Comma:
        emit prevSnapshot();
        break;
      case Qt::Key_Period:
        emit nextSnapshot();
        break;
      case Qt::Key_A:
        if(mods == Qt::ControlModifier)
          emit prevSnapshot();
        break;
      case Qt::Key_D:
        if(mods == Qt::ControlModifier)
          emit nextSnapshot();
        break;
      case Qt::Key_W:
        emit play();
        break;
      case Qt::Key_S:
        emit pause();
        break;
    }
  }
  return false;
}

void VisionWindow::setStreaming(bool value) {
  streaming_ = value;
}

void VisionWindow::loadConfig(const ToolConfig& tconfig) {
  config_ = tconfig.visionConfig;
  tabs->setCurrentIndex(config_.tab);
  cbxOverlay->setChecked(config_.all);
  cbxHorizon->setChecked(config_.horizon);
  cbxTooltip->setChecked(config_.tooltip);
  cbxCalibration->setChecked(config_.calibration);
  cbxCheckerboard->setChecked(config_.checkerboard);
}

void VisionWindow::saveConfig(ToolConfig& tconfig) {
  tconfig.visionConfig = config_;
}

void VisionWindow::controlsChanged() {
  if(loading_) return;
  config_.tab = tabs->currentIndex();
  config_.all = cbxOverlay->isChecked();
  config_.horizon = cbxHorizon->isChecked();
  config_.tooltip = cbxTooltip->isChecked();
  config_.calibration = cbxCalibration->isChecked();
  config_.checkerboard = cbxCheckerboard->isChecked();
  ConfigWindow::saveConfig();
  redrawImages();
}

void VisionWindow::showEvent(QShowEvent*) {
  if(frame_ > 0)
    emit newLogFrame(frame_);
  if(log_ != nullptr)
    emit newLogLoaded(log_);
}
    
void VisionWindow::wheelEvent(QWheelEvent* event) {
  if(!bigImage->underMouse()) return;
  int deg = event->delta() / 8;
  int steps = deg / 15;
  if(steps > 0) emit nextSnapshot();
  else if(steps < 0) emit prevSnapshot();
  event->accept();
}
