#include <QtGui>
#include "ExtrinsicCalibrationWidget.h"

class CalibrationItem : public QListWidgetItem {
  public:
    QLabel* name;
    QDoubleSpinBox* value;
    QLayout* layout;
    QWidget* widget;
    int index;
    CalibrationItem(std::string nameStr, double val, int idx) {
      index = idx;
      layout = new QHBoxLayout;
      name = new QLabel;
      name->setText(QString::fromStdString(nameStr));
      layout->addWidget(name);
      value = new QDoubleSpinBox;
      value->setValue(val);
      value->setMinimum(-100.0);
      value->setMaximum(100.0);
      value->setSingleStep(0.1);
      layout->addWidget(value);
      widget = new QWidget;
      layout->setSizeConstraint(QLayout::SetFixedSize);
      widget->setLayout(layout);
      setSizeHint(widget->sizeHint());
    }
};

class SampleItem : public QListWidgetItem {
  private:
    int _x, _y;
  public:
    SampleItem();
    SampleItem(int,int);
};

SampleItem::SampleItem() : QListWidgetItem() {}

SampleItem::SampleItem(int x, int y){
  _x = x;
  _y = y;
  QString text = "X=" + QString::number(x) + ", Y=" + QString::number(y);
  setText(text);
}

ExtrinsicCalibrationWidget::ExtrinsicCalibrationWidget(QWidget* parent) : QWidget(parent) {
  setupUi(this);

  connect(samplesButton, SIGNAL(clicked()), this, SLOT(takeSamples()));
  connect(startButton, SIGNAL(clicked()), this, SLOT(optimizeCalibration()));
  connect(stopButton, SIGNAL(clicked()), this, SLOT(stopCalibration()));
  connect(resetButton, SIGNAL(clicked()), this, SLOT(resetCalibration()));
  connect(clearButton,SIGNAL(clicked()), this, SLOT(clear()));
  //connect(resetButton,SIGNAL(clicked()), this, SLOT(resetParameters()));
  connect(saveButton,SIGNAL(clicked()), this, SLOT(save()));
  connect(saveAsButton,SIGNAL(clicked()), this, SLOT(saveAs()));
  connect(loadButton,SIGNAL(clicked()), this, SLOT(load()));

  auto controls = vector<QDoubleSpinBox*> {
    rhPitch, rhRoll, rhYP, lhPitch, lhRoll, lhYP,
    poseX, poseY, poseTheta,
    topTilt, topRoll, topYaw, topFOVx, topFOVy,
    bottomTilt, bottomRoll, bottomYaw, bottomFOVx, bottomFOVy
  };
  for(auto c : controls)
    connect(c,SIGNAL(valueChanged(double)),this,SIGNAL(calibrationsUpdated()));


  connect(dimensionCheck,SIGNAL(toggled(bool)),dimensionBox,SLOT(setVisible(bool)));
  connect(sensorCheck,SIGNAL(toggled(bool)),sensorBox,SLOT(setVisible(bool)));
  connect(jointCheck,SIGNAL(toggled(bool)),jointBox,SLOT(setVisible(bool)));
  connect(rdoLeft,SIGNAL(toggled(bool)), this, SIGNAL(calibrationsUpdated()));

  connect(this, SIGNAL(calibrationsUpdated()), this, SLOT(handleUpdatedCalibrations()));

  calibration_file_ = (QString(getenv("NAO_HOME")) + "/data/default_calibration.cal").toUtf8().constData();
  initializeItems();
  sensorBox->setVisible(false);
  dimensionBox->setVisible(false);
  jointBox->setVisible(false);
  loadCalibration(calibration_file_);
  currentCamera_ = Camera::TOP;
}

void ExtrinsicCalibrationWidget::toggleList(int state, QWidget* w) {
  w->setVisible((bool)state);
}

void ExtrinsicCalibrationWidget::setWorldObjectBlock(WorldObjectBlock* block) {
  world_object_block_ = block;
}

void ExtrinsicCalibrationWidget::addSample(Sample s){
  SampleItem* item = new SampleItem(s.x,s.y);
  samplesList->addItem(item);
  samples_.push_back(s);
}

void ExtrinsicCalibrationWidget::initializeItems() {
  RobotCalibration cal;
  for(int i = 0; i < RobotDimensions::NUM_DIMENSIONS; i++) {
    if(i >= RobotDimensions::tiltOffsetToBottomCamera && i <= RobotDimensions::yawOffsetToTopCamera) continue;
    CalibrationItem* item = new CalibrationItem(DimensionNames[i], cal.dimensionValues_[i], i);
    connect(item->value, SIGNAL(valueChanged(double)), this, SIGNAL(calibrationsUpdated()));
    dimensionList->addItem(item);
    dimensionList->setItemWidget(item, item->widget);
  }
  for(int i = 0; i < NUM_SENSORS; i++) {
    CalibrationItem* item = new CalibrationItem(SensorNames[i], cal.sensorValues_[i], i);
    connect(item->value, SIGNAL(valueChanged(double)), this, SIGNAL(calibrationsUpdated()));
    sensorList->addItem(item);
    sensorList->setItemWidget(item, item->widget);
  }
  for(int i = 0; i < NUM_JOINTS; i++) {
    if(i >= LHipYawPitch && i <= LHipPitch) continue;
    if(i >= RHipYawPitch && i <= RHipPitch) continue;
    CalibrationItem* item = new CalibrationItem(JointNames[i], cal.jointValues_[i], i);
    connect(item->value, SIGNAL(valueChanged(double)), this, SIGNAL(calibrationsUpdated()));
    jointList->addItem(item);
    jointList->setItemWidget(item, item->widget);
  }
}

void ExtrinsicCalibrationWidget::loadCalibration(const RobotCalibration& cal, bool includePose) {
  if(includePose) {
    poseX->setValue(cal.poseX);
    poseY->setValue(cal.poseY);
    poseTheta->setValue(cal.poseTheta * RAD_T_DEG);
  }
  topFOVx->setValue(cal.topFOVx * RAD_T_DEG);
  topFOVy->setValue(cal.topFOVy * RAD_T_DEG);
  topTilt->setValue(cal.dimensionValues_[RobotDimensions::tiltOffsetToTopCamera] * RAD_T_DEG);
  topRoll->setValue(cal.dimensionValues_[RobotDimensions::rollOffsetToTopCamera] * RAD_T_DEG);
  topYaw->setValue(cal.dimensionValues_[RobotDimensions::yawOffsetToTopCamera] * RAD_T_DEG);
  bottomFOVx->setValue(cal.bottomFOVx * RAD_T_DEG);
  bottomFOVy->setValue(cal.bottomFOVy * RAD_T_DEG);
  bottomTilt->setValue(cal.dimensionValues_[RobotDimensions::tiltOffsetToBottomCamera] * RAD_T_DEG);
  bottomRoll->setValue(cal.dimensionValues_[RobotDimensions::rollOffsetToBottomCamera] * RAD_T_DEG);
  bottomYaw->setValue(cal.dimensionValues_[RobotDimensions::yawOffsetToBottomCamera] * RAD_T_DEG);
  lhRoll->setValue(cal.jointValues_[LHipRoll] * RAD_T_DEG);
  lhPitch->setValue(cal.jointValues_[LHipPitch] * RAD_T_DEG);
  lhYP->setValue(cal.jointValues_[LHipYawPitch] * RAD_T_DEG);
  rhRoll->setValue(cal.jointValues_[RHipRoll] * RAD_T_DEG);
  rhPitch->setValue(cal.jointValues_[RHipPitch] * RAD_T_DEG);
  rhYP->setValue(cal.jointValues_[RHipYawPitch] * RAD_T_DEG);
  rdoLeft->setChecked(cal.useLeft);
  rdoRight->setChecked(!cal.useLeft);
  for(int i = 0; i < sensorList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)sensorList->item(i);
    item->value->setValue(cal.sensorValues_[item->index]);
  }
  for(int i = 0; i < jointList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)jointList->item(i);
    item->value->setValue(cal.jointValues_[item->index] * RAD_T_DEG);
  }
  for(int i = 0; i < dimensionList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)dimensionList->item(i);
    float value = cal.dimensionValues_[item->index];
    if(RobotDimensions::isAngle_[item->index]) value *= RAD_T_DEG;
    item->value->setValue(value);
  }
}

RobotCalibration ExtrinsicCalibrationWidget::getCalibration(bool includePose) const {
  RobotCalibration cal;
  if(includePose) {
    cal.poseX = poseX->value();
    cal.poseY = poseY->value();
    cal.poseTheta = poseTheta->value() * DEG_T_RAD;
  }
  cal.topFOVx = topFOVx->value() * DEG_T_RAD;
  cal.topFOVy = topFOVy->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::tiltOffsetToTopCamera] = topTilt->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::rollOffsetToTopCamera] = topRoll->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::yawOffsetToTopCamera] = topYaw->value() * DEG_T_RAD;
  cal.bottomFOVx = bottomFOVx->value() * DEG_T_RAD;
  cal.bottomFOVy = bottomFOVy->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::tiltOffsetToBottomCamera] = bottomTilt->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::rollOffsetToBottomCamera] = bottomRoll->value() * DEG_T_RAD;
  cal.dimensionValues_[RobotDimensions::yawOffsetToBottomCamera] = bottomYaw->value() * DEG_T_RAD;
  
  cal.jointValues_[LHipRoll] = lhRoll->value() * DEG_T_RAD;
  cal.jointValues_[LHipPitch] = lhPitch->value() * DEG_T_RAD;
  cal.jointValues_[LHipYawPitch] = lhYP->value() * DEG_T_RAD;
  cal.jointValues_[RHipRoll] = rhRoll->value() * DEG_T_RAD;
  cal.jointValues_[RHipPitch] = rhPitch->value() * DEG_T_RAD;
  cal.jointValues_[RHipYawPitch] = rhYP->value() * DEG_T_RAD;

  cal.useLeft = rdoLeft->isChecked();
  for(int i = 0; i < sensorList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)sensorList->item(i);
    cal.sensorValues_[item->index] = item->value->value();
  }
  for(int i = 0; i < jointList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)jointList->item(i);
    cal.jointValues_[item->index] = item->value->value() * DEG_T_RAD;
  }
  for(int i = 0; i < dimensionList->count(); i++) {
    CalibrationItem* item = (CalibrationItem*)dimensionList->item(i);
    float value = item->value->value();
    if(RobotDimensions::isAngle_[item->index]) value *= DEG_T_RAD;
    cal.dimensionValues_[item->index] = value;
  }
  return cal;
}

void ExtrinsicCalibrationWidget::saveCalibration(std::string file) {
  RobotCalibration cal = getCalibration();
  cal.saveToFile(file);
}

void ExtrinsicCalibrationWidget::loadCalibration(std::string file) {
  RobotCalibration cal;
  if(!cal.loadFromFile(file)) return;
  loadCalibration(cal, false);
}

void ExtrinsicCalibrationWidget::resetParameters(){
  loadCalibration(calibration_file_);
}

void ExtrinsicCalibrationWidget::clear(){
  samplesList->clear();
  samples_.clear();
}

void ExtrinsicCalibrationWidget::save(){
  std::cout << "Saving calibration to file: " << calibration_file_ << "...";
  saveCalibration(calibration_file_);
  std::cout << "done!\n";
}

void ExtrinsicCalibrationWidget::saveAs(){
  QString file = QFileDialog::getSaveFileName(this, tr("Save Camera Calibration"),
    QString(getenv("NAO_HOME")) + "/data",
    tr("Camera Calibration Files (*.cal)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty()) return;
  calibration_file_ = file.toUtf8().constData();
  save();
}

void ExtrinsicCalibrationWidget::load(){
  QString file = QFileDialog::getOpenFileName(this, tr("Open Camera Calibration"),
    QString(getenv("NAO_HOME")) + "/data",
    tr("Camera Calibration Files (*.cal)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty())
    return;

  calibration_file_ = file.toUtf8().constData();
  std::cout << "Loading calibration from file: " << calibration_file_ << "...";
  loadCalibration(calibration_file_);
  std::cout << "done!\n";
}

void ExtrinsicCalibrationWidget::stopCalibration() {
  calibrator_.stop();
  loadCalibration(calibrator_.getCalibration(), false);
}

void ExtrinsicCalibrationWidget::handleUpdatedCalibrations() {
  calibrator_.left() = rdoLeft->isChecked();
}

void ExtrinsicCalibrationWidget::takeSamples() {
  calibrator_.takeSamples(log_);
}

void ExtrinsicCalibrationWidget::optimizeCalibration() {
  auto callback = [&] {
    loadCalibration(calibrator_.getCalibration(), false);
  };
  calibrator_.start(iterations->value(), callback);
}

void ExtrinsicCalibrationWidget::resetCalibration() {
  calibrator_.reset();
}

std::vector<Sample> ExtrinsicCalibrationWidget::getSamples() const {
  return samples_;
}

void ExtrinsicCalibrationWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom){
  topProcessor_ = top;
  bottomProcessor_ = bottom;
}

void ExtrinsicCalibrationWidget::setCurrentCamera(Camera::Type camera){
  currentCamera_ = camera;
}

void ExtrinsicCalibrationWidget::handleNewLogLoaded(LogViewer* log){
  log_ = log;
}

void ExtrinsicCalibrationWidget::handleNewLogFrame(int frame){
  currentFrame_ = frame;
}

void ExtrinsicCalibrationWidget::handleNewStreamFrame() {
}
