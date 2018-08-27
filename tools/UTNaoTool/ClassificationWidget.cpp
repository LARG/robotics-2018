#include <tool/ClassificationWidget.h>
#include <vision/ImageProcessor.h>
#define generateForColor(c) (flags & (1 << c))

ClassificationWidget::ClassificationWidget(QWidget* parent) : QWidget(parent), log_(NULL) {
    setupUi(this);

    connect (undefButton, SIGNAL(clicked()), this, SLOT(setUndef()) );
    connect (greenButton, SIGNAL(clicked()), this, SLOT(setGreen()) );
    connect (whiteButton, SIGNAL(clicked()), this, SLOT(setWhite()) );
    connect (orangeButton, SIGNAL(clicked()), this, SLOT(setOrange()) );
    connect (yellowButton, SIGNAL(clicked()), this, SLOT(setGoalYellow()) );
    connect (blueButton, SIGNAL(clicked()), this, SLOT(setGoalBlue()) );
    connect (pinkButton, SIGNAL(clicked()), this, SLOT(setRobotPink()) );
    connect (robotWhiteButton, SIGNAL(clicked()), this, SLOT(setRobotWhite()) );

    connect (generateButton, SIGNAL(clicked()), this, SLOT(generateColorTable()));

    strCol[c_UNDEFINED]="Undefined";
    strCol[c_FIELD_GREEN]="Field Green";
    strCol[c_WHITE]="White";
    strCol[c_ORANGE]="Orange";
    strCol[c_PINK]="Pink";
    strCol[c_BLUE]="Blue";
    strCol[c_YELLOW]="Yellow";
    strCol[c_ROBOT_WHITE] = "Robot White";

    for (int i=0; i<Color::NUM_Colors; i++) {
        colorCombo->addItem(strCol[i]);
    }
    maxFrames_ = 0;
}

void ClassificationWidget::setUndef() {
  colorCombo->setCurrentIndex(c_UNDEFINED);
}
void ClassificationWidget::setGreen() {
  colorCombo->setCurrentIndex(c_FIELD_GREEN);
}
void ClassificationWidget::setWhite() {
  colorCombo->setCurrentIndex(c_WHITE);
}
void ClassificationWidget::setOrange() {
  colorCombo->setCurrentIndex(c_ORANGE);
}
void ClassificationWidget::setGoalBlue() {
  colorCombo->setCurrentIndex(c_BLUE);
}
void ClassificationWidget::setGoalYellow() {
  colorCombo->setCurrentIndex(c_YELLOW);
}
void ClassificationWidget::setRobotPink() {
  colorCombo->setCurrentIndex(c_PINK);
}
void ClassificationWidget::setRobotWhite() {
  colorCombo->setCurrentIndex(c_ROBOT_WHITE);
}

void ClassificationWidget::generateColorTable() {
  int flags = getColorFlags();
  std::cout << "Generating color table...";
  ImageProcessor* processor = (currentCamera_ == Camera::TOP ? topProcessor_ : bottomProcessor_);
  unsigned char* colorTable = processor->getColorTable();
  unsigned char original[LUT_SIZE];
  int width = processor->getImageWidth();
  memcpy(original, colorTable, LUT_SIZE);
  for(int y = 0; y < 256; y+=2) {
    for(int u = 0; u < 256; u+=2) {
      for(int v = 0; v < 256; v+=2) {
        Color assignment = ColorTableMethods::yuv2color(colorTable, y, u, v);
        if(generateForColor(assignment))
          ColorTableMethods::assignColor(colorTable, y, u, v, c_UNDEFINED);
      }
    }
  }
  int yrad = yradius->value(), urad = uradius->value(), vrad = vradius->value();
  auto images = (currentCamera_ == Camera::TOP ? log_->getRawTopImages() : log_->getRawBottomImages());
  std::vector<ImageParams> iparams = (currentCamera_ == Camera::TOP ? log_->getTopParams() : log_->getBottomParams());
  for(int frame = 0; frame < maxFrames_; frame++) {
    auto image = images[frame];
    int count = annotations_.size();
    for(int i = 0; i < count; i++){
      VisionAnnotation* annotation = annotations_[i];
      Color c = annotation->getColor();
      if(annotation->getCamera() != currentCamera_) continue;
      if(!annotation->isInFrame(frame)) continue;
      if(!annotation->isSample()) continue;
      if(!generateForColor(c)) continue;
      std::vector<Point> points = annotation->getEnclosedPoints(frame);
      int pcount = points.size();
      for(int j=0; j < pcount; j++){
        Point p = points[j];
        if(p.x >= iparams[frame].width || p.y >= iparams[frame].height) continue;
        Color current = ColorTableMethods::xy2color(image.data(), original, p.x, p.y, width);
        if(!generateForColor(current)) continue;
        ColorTableMethods::assignColor(image.data(), colorTable, p.x, p.y, width, c, yrad, urad, vrad);
      }
    }
  }
  std::cout << "done\n";
  emit colorTableGenerated();
}

void ClassificationWidget::setCurrentCamera(Camera::Type camera){
  currentCamera_ = camera;
}

void ClassificationWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom){
  topProcessor_ = top;
  bottomProcessor_ = bottom;
}

void ClassificationWidget::handleNewLogLoaded(LogViewer* log){
  log_ = log;
  maxFrames_ = log_->size();
}

void ClassificationWidget::setAnnotations(std::vector<VisionAnnotation*> annotations){
  annotations_ = annotations;
}

int ClassificationWidget::getColorFlags() {
  int flags = 0;
  flags = flags | (1 << c_UNDEFINED);
  if(greenBox->isChecked())
    flags = flags | ( 1 << c_FIELD_GREEN);
  if(orangeBox->isChecked())
    flags = flags | ( 1 << c_ORANGE);
  if(yellowBox->isChecked())
    flags = flags | ( 1 << c_YELLOW);
  if(whiteBox->isChecked())
    flags = flags | ( 1 << c_WHITE);
  if(robotWhiteBox->isChecked())
    flags = flags | ( 1 << c_ROBOT_WHITE);
  if(pinkBox->isChecked())
    flags = flags | ( 1 << c_PINK);
  if(blueBox->isChecked())
    flags = flags | ( 1 << c_BLUE);
  return flags;
}
