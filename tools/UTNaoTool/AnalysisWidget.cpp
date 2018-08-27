#include <tool/AnalysisWidget.h>
#include <localization/LocalizationModule.h>
#include <vision/ImageProcessor.h>

AnalysisWidget::AnalysisWidget(QWidget* parent) : QWidget(parent), log_(NULL), core_(NULL) {
  setupUi(this);

  colorStrings[c_UNDEFINED]="Undefined";
  colorStrings[c_FIELD_GREEN]="Field Green";
  colorStrings[c_WHITE]="White";
  colorStrings[c_ORANGE]="Orange";
  colorStrings[c_PINK]="Pink";
  colorStrings[c_BLUE]="Blue";
  colorStrings[c_YELLOW]="Goal Posts";
  colorStrings[c_ROBOT_WHITE] = "Robot White";

  for (int i=1; i<Color::NUM_Colors; i++) {
    colorBox->addItem(colorStrings[i]);
  }

  connect(analyzeButton, SIGNAL(clicked()), this, SLOT(analyze()));
  connect(pruneButton, SIGNAL(clicked()), this, SLOT(prune()));
  connect(undoButton, SIGNAL(clicked()), this, SLOT(undo()));
  connect (colorBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(colorBoxIndexChanged(const QString &)));

  displayBallInfo(false);
  displayGoalInfo(false);

}

void AnalysisWidget::displayBallInfo(bool show){
  txtFalseCandidates->setVisible(show);
  txtFalseBalls->setVisible(show);
  txtMissingBalls->setVisible(show);
  lblFalseCandidates->setVisible(show);
  lblFalseBalls->setVisible(show);
  lblMissingBalls->setVisible(show);
  txtBallStats->setVisible(show);
 
}

void AnalysisWidget::displayGoalInfo(bool show){
  lblTrueGoals->setVisible(show);
  txtTrueGoals->setVisible(show);
  lblFalseGoals->setVisible(show);
  txtFalseGoals->setVisible(show);
  lblMissingGoals->setVisible(show);
  txtMissingGoals->setVisible(show);
  txtGoalStats->setVisible(show);
}

void AnalysisWidget::analyze(){
  analyzer_.setAnnotations(annotations_);
  ImageProcessor* processor = (camera_ == Camera::TOP ? topProcessor_ : bottomProcessor_);
  unsigned char* colorTable = processor->getColorTable();
  analyzer_.setColorTable(colorTable);
  analyzer_.readImageData();
  fpcText->setText(QString::number(analyzer_.falsePositiveCount(selectedColor_)));
  fprText->setText(QString::number(analyzer_.falsePositiveRate(selectedColor_)));
  fncText->setText(QString::number(analyzer_.falseNegativeCount(selectedColor_)));
  fnrText->setText(QString::number(analyzer_.falseNegativeRate(selectedColor_)));
  // Doing this so core gets set
  emit memoryChanged();
  if(selectedColor_ == c_ORANGE) {
    ballstats stats = getBallStatistics();
    txtFalseBalls->setText(QString::number(stats.falseBalls));
    txtMissingBalls->setText(QString::number(stats.missingBalls));
    txtFalseCandidates->setText(QString::number(stats.falseCandidates));
  }
  else if (selectedColor_ == c_YELLOW){
    goalstats stats = getGoalStatistics();
    txtFalseGoals->setText(QString::number(stats.falsePosts));
    txtMissingGoals->setText(QString::number(stats.missingPosts));
    txtTrueGoals->setText(QString::number(stats.truePosts));
  }
  pointCountText->setText(QString::number(analyzer_.colorTablePointCount(selectedColor_)));
}

void AnalysisWidget::handleNewLogLoaded(LogViewer* log){
  analyzer_.setLog(log);
  log_ = log;
}

void AnalysisWidget::setAnnotations(std::vector<VisionAnnotation*> annotations){
  annotations_ = annotations;
}

void AnalysisWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom){
  topProcessor_ = top;
  bottomProcessor_ = bottom;
}

void AnalysisWidget::setCurrentCamera(Camera::Type camera){
  analyzer_.setCamera(camera);
  camera_ = camera;
}

void AnalysisWidget::colorBoxIndexChanged(const QString& text) {
  (void)text; // kill warning
  int index = colorBox->currentIndex();
  selectedColor_ = (Color)(index + 1);
  if(selectedColor_ == c_ORANGE) {
    displayBallInfo(true);
    displayGoalInfo(false);
  }
  else if (selectedColor_ == c_YELLOW){
    displayBallInfo(false);
    displayGoalInfo(true);
      
  }
  else {
    displayBallInfo(false);
    displayGoalInfo(false);
  }

}

void AnalysisWidget::prune() {
  analyzer_.setAnnotations(annotations_);
  ImageProcessor* processor = (camera_ == Camera::TOP ? topProcessor_ : bottomProcessor_);
  unsigned char* colorTable = processor->getColorTable();
  analyzer_.setColorTable(colorTable);
  float amount = (float)prunePercentBox->value() / 100;
  analyzer_.removeCriticalPoints(selectedColor_, amount);
  emit colorTableGenerated();
  analyze();
  undoButton->setEnabled(true);
  std::cout << "Pruned bad assignments for " << colorStrings[selectedColor_].toStdString() << "\n";
}

void AnalysisWidget::undo() {
  analyzer_.undo();
  emit colorTableGenerated();
  analyze();
  if(!analyzer_.hasUndo())
    undoButton->setEnabled(false);
}

void AnalysisWidget::handleColorTableGenerated(){
  analyzer_.clear();
  undoButton->setEnabled(false);
}


goalstats AnalysisWidget::getGoalStatistics() {
  return goalstats();
}

ballstats AnalysisWidget::getBallStatistics() {
  return ballstats();
}

void AnalysisWidget::setCore(VisionCore* core){
  core_ = core;
}

void AnalysisWidget::handleNewLogFrame(int frame) {
  currentFrame_ = frame;
}
