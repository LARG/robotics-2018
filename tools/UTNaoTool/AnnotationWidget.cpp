#include <tool/AnnotationWidget.h>
#include <regex>
#include <common/Util.h>

AnnotationWidget::AnnotationWidget(QWidget* parent) : ConfigWidget(parent), selectedAnnotation_(nullptr) {
  setupUi(this);

  for(auto name : names_Color())
    colorBox->addItem(QString::fromStdString(name));

  for(auto name : SelectionTypeMethods::names())
    selectionTypeBox->addItem(QString::fromStdString(name));

  for(auto name : Camera::names())
    cameraBox->addItem(QString::fromStdString(name));

  connect (colorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged()));
  connect (cameraBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged()));
  connect (selectionTypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged()));
  connect (cbxAuto, SIGNAL(toggled(bool)), this, SLOT(controlsChanged()));

  connect (annotationList, SIGNAL(itemSelectionChanged()), this, SLOT(annotationSelected()));
  connect (selectionList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionSelected()));
  connect (selectionTypeBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(selectionBoxIndexChanged(const QString &)));
  connect (cameraBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(cameraBoxIndexChanged(const QString &)));
  connect (colorBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(colorBoxIndexChanged(const QString &)));
  connect (cbxEnable, SIGNAL(toggled(bool)), this, SLOT(enableToggled(bool)));

  //connect (insertButton, SIGNAL(clicked()), this, SLOT(insert()));
  connect (updateButton, SIGNAL(clicked()), this, SLOT(update()));
  connect (deleteAnnotButton, SIGNAL(clicked()), this, SLOT(deleteAnnotation()));
  connect (deleteSelectionButton, SIGNAL(clicked()), this, SLOT(deleteSelection()));

  connect (greenBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (orangeBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (yellowBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (whiteBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (robotWhiteBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (pinkBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
  connect (blueBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));

  connect (saveButton, SIGNAL(clicked()), this, SLOT(saveToFile()));

  connect (clearMovementsButton, SIGNAL(clicked()), this, SLOT(clearMovements()));

  connect (rdoAll, SIGNAL(toggled(bool)), this, SLOT(updateFrameSelection()));
  connect (rdoCurrent, SIGNAL(toggled(bool)), this, SLOT(updateFrameSelection()));
  connect (rdoCustom, SIGNAL(toggled(bool)), this, SLOT(updateFrameSelection()));

  currentCamera_ = selectedCamera_ = Camera::TOP;
}

void AnnotationWidget::setMaxFrames(int frames){
  maxFrames_ = frames;
  updateFrameSelection();
  minBox->setMaximum(frames - 1);
  maxBox->setMaximum(frames - 1);
}

void AnnotationWidget::selected(Selection* selection){
  if(!enabled()) return;

  std::string name = util::trim(annotname->text().toStdString());
  auto it = annotations_.find(name);
  if(cbxAuto->isChecked()) {
    // Pick a unique name for the annotation by adding a counter
    // to the end. If the counter is already present in the current
    // name, extract the base and re-add the counter.
    std::regex re("(.*) \\d+$");
    std::smatch match;
    std::string base = name;
    if(std::regex_search(name, match, re) && match.size() > 1)
      base = match.str(1);
    for(int i = 0; it != annotations_.end(); i++) {
      name = base + " " + std::to_string(i);
      it = annotations_.find(name);
    }
  }
  VisionAnnotation* annotation;
  if(it == annotations_.end()) {
    annotation = new VisionAnnotation(name);
    annotations_[annotation->getName()] = annotation;
    annotation->setMaxFrame(maxBox->value());
    annotation->setMinFrame(minBox->value());
    annotation->setColor(selectedColor_);
    annotation->setCamera(selectedCamera_);
    annotation->setSample(cbxSampleSet->isChecked());
    auto item = new AnnotationListWidgetItem(annotation);
    annotationItems_[annotation] = item;
    annotationList->addItem(item);
    if(selectedAnnotation_ == nullptr) {
      selectedAnnotation_ = annotation;
      annotationList->setCurrentItem(item);
    }
  } else {
    annotation = it->second;
  }

  annotation->addSelection(selection);
  if(selectedAnnotation_ == annotation)
    loadSelections(annotation);
  emit setCurrentAnnotations(getAnnotations());
  emit setCurrentLogFrame(annotation->getMinFrame());
  redrawCurrentSelections();
}

std::vector<VisionAnnotation*> AnnotationWidget::getAnnotations() {
  std::vector<VisionAnnotation*> annotations;
  int count = annotationList->count();
  for(int i = 0; i < count; i++){
    QListWidgetItem* item = annotationList->item(i);
    AnnotationListWidgetItem* aitem = static_cast<AnnotationListWidgetItem*>(item);
    VisionAnnotation* annotation = aitem->getAnnotation();
    annotations.push_back(annotation);
  }
  return annotations;
}

std::vector<Selection*> AnnotationWidget::getSelections() {
  std::vector<Selection*> selections;
  int count = selectionList->count();
  for(int i = 0; i < count; i++){
    QListWidgetItem* item = selectionList->item(i);
    SelectionListWidgetItem* sitem = static_cast<SelectionListWidgetItem*>(item);
    Selection* selection = sitem->getSelection();
    selections.push_back(selection);
  }
  return selections;
}

void AnnotationWidget::annotationSelected() {
  if(selecting_) return;
  if(annotationList->selectedItems().count() == 0) {
    clearSelections();
    return;
  }
  annotationList->setSelectionMode(QAbstractItemView::SingleSelection);
  QListWidgetItem* item = annotationList->selectedItems()[0];
  AnnotationListWidgetItem* aitem = static_cast<AnnotationListWidgetItem*>(item);
  VisionAnnotation* annotation = aitem->getAnnotation();
  loadSelections(annotation);
  loadChoices(annotation);
  emit setCurrentLogFrame(annotation->getMinFrame());
}

void AnnotationWidget::selectionSelected() {
  if(selectionList->selectedItems().count() == 0) {
    return;
  }
  selectionList->setSelectionMode(QAbstractItemView::SingleSelection);
  for(auto kvp : annotations_) {
    auto a = kvp.second;
    for(auto s : a->getSelections())
      s->setHovered(false);
  }
  auto sitem = static_cast<SelectionListWidgetItem*>(selectionList->selectedItems()[0]);
  sitem->getSelection()->setHovered(true);
  for(auto kvp : annotations_) {
    auto a = kvp.second;
    if(a->hasSelection(sitem->getSelection())) {
      emit setCurrentLogFrame(a->getMinFrame());
      break;
    }
  }
  redrawCurrentSelections();
}

void AnnotationWidget::loadChoices(VisionAnnotation* annotation) {
  annotname->setText(QString::fromStdString(annotation->getName()));
  colorBox->setCurrentIndex(annotation->getColor());
  cameraBox->setCurrentIndex(annotation->getCamera());
  minBox->setValue(annotation->getMinFrame());
  maxBox->setValue(annotation->getMaxFrame());
  cbxSampleSet->setChecked(annotation->isSample());
}

void AnnotationWidget::loadSelections(VisionAnnotation* annotation) {
  if(selecting_) return;
  clearSelections();
  for(auto s : annotation->getSelections()) {
    auto item = new SelectionListWidgetItem(s);
    selectionList->addItem(item);
    selectionItems_[s] = item;
  }
  selectedAnnotation_ = annotation;
}

void AnnotationWidget::clearSelections() {
  selectionList->clear();
  selectionItems_.clear();
  selectedAnnotation_ = nullptr;
}

void AnnotationWidget::colorBoxIndexChanged(const QString&) {
  int index = colorBox->currentIndex();
  selectedColor_ = (Color)index;
}

void AnnotationWidget::cameraBoxIndexChanged(const QString& label) {
  std::string str = label.toStdString();
  if(str == "Top")
    selectedCamera_ = Camera::TOP;
  else
    selectedCamera_ = Camera::BOTTOM;
}


void AnnotationWidget::selectionBoxIndexChanged(const QString& label) {
  auto evalue = SelectionTypeMethods::fromName(label.toStdString());
  emit selectionTypeChanged(evalue);
}

void AnnotationWidget::enableToggled(bool value) {
  emit selectionEnabled(value);
  emit selectionTypeChanged(config_.selection_type);
}

void AnnotationWidget::handleNewLogFrame(int frame) {
  currentFrame_ = frame;
  std::vector<Selection*> currentSelections;
  int count = annotationList->count();
  for(int i = 0; i < count; i++){
    QListWidgetItem* item = annotationList->item(i);
    AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
    VisionAnnotation* annotation = aitem->getAnnotation();
    annotation->setCurrentFrame(currentFrame_);
    if(filterAnnotation(annotation)) continue;
    const std::vector<Selection*> selections = annotation->getSelections();
    int scount = selections.size();
    for(int j=0; j < scount; j++) {
      currentSelections.push_back(selections[j]);
    }
  }
  updateFrameSelection();
  emit setCurrentSelections(currentSelections);
}

void AnnotationWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom) {
  topProcessor_ = top;
  bottomProcessor_ = bottom;
}

void AnnotationWidget::setCurrentCamera(Camera::Type camera){
  currentCamera_ = camera;
  cameraBox->setCurrentIndex(camera == Camera::TOP ? 0 : 1);
  redrawCurrentSelections();
}

void AnnotationWidget::handleNewLogLoaded(LogViewer* log){
  log_ = log;
  setMaxFrames(log_->size());
  annotations_.clear();
  annotationList->clear();
  clearSelections();

  AnnotationGroup group;
  if(!group.load(log)) {
    redrawCurrentSelections();
    return;
  }
  for(auto a : group.getVisionAnnotations()) {
    auto item = new AnnotationListWidgetItem(a);
    annotationList->addItem(item);
    annotations_[a->getName()] = a;
    annotationItems_[a] = item;
  }
  updateFrameSelection();
  redrawCurrentSelections();
  emit setCurrentAnnotations(getAnnotations());
}

void AnnotationWidget::insert() {

}

void AnnotationWidget::update() {
  if(annotationList->selectedItems().count() == 0)
    return;
  QListWidgetItem* item = annotationList->selectedItems()[0];
  AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
  VisionAnnotation* annotation = aitem->getAnnotation();
  annotation->setName(annotname->text().toStdString());
  annotation->setMaxFrame(maxBox->value());
  annotation->setMinFrame(minBox->value());
  annotation->setColor(selectedColor_);
  annotation->setCamera(selectedCamera_);
  annotation->setSample(cbxSampleSet->isChecked());
  aitem->resetText();
  redrawCurrentSelections();
}

void AnnotationWidget::deleteAnnotation() {
  if(annotationList->selectedItems().count() == 0)
    return;
  QListWidgetItem* item = annotationList->selectedItems()[0];
  AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
  VisionAnnotation* annotation = aitem->getAnnotation();
  std::map<std::string, VisionAnnotation*>::iterator it = annotations_.find(annotation->getName());
  annotations_.erase(it, annotations_.end());
  delete item;
  delete annotation;
  redrawCurrentSelections();
}

void AnnotationWidget::deleteSelection() {
  if(annotationList->selectedItems().count() == 0)
    return;
  if(selectionList->selectedItems().count() == 0)
    return;
  QListWidgetItem* item = annotationList->selectedItems()[0];
  AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
  VisionAnnotation* annotation = aitem->getAnnotation();

  QListWidgetItem* item2 = selectionList->selectedItems()[0];
  SelectionListWidgetItem* sitem = (SelectionListWidgetItem*)item2;
  Selection* selection = sitem->getSelection();

  annotation->removeSelection(selection);
  selectionList->removeItemWidget(item2);
  delete selection;
  delete item2;
  redrawCurrentSelections();
}

bool AnnotationWidget::filterAnnotation(VisionAnnotation* annotation){
  if(annotation->getCamera() != currentCamera_) return true;
  if(!annotation->isInFrame(currentFrame_)) return true;
  if(!greenBox->isChecked() && annotation->getColor() == c_FIELD_GREEN)
    return true;
  if(!orangeBox->isChecked() && annotation->getColor() == c_ORANGE)
    return true;
  if(!yellowBox->isChecked() && annotation->getColor() == c_YELLOW)
    return true;
  if(!whiteBox->isChecked() && annotation->getColor() == c_WHITE)
    return true;
  if(!robotWhiteBox->isChecked() && annotation->getColor() == c_ROBOT_WHITE)
    return true;
  if(!pinkBox->isChecked() && annotation->getColor() == c_PINK)
    return true;
  if(!blueBox->isChecked() && annotation->getColor() == c_BLUE)
    return true;
  return false;
}

void AnnotationWidget::redrawCurrentSelections() {
  handleNewLogFrame(currentFrame_);
}

void AnnotationWidget::saveToFile() {
  AnnotationGroup group(getAnnotations());
  group.save(log_);
  printf("Saved annotations to: %s\n", group.path(log_->directory()).c_str());
}

void AnnotationWidget::clearMovements() {
  if(selectedAnnotation_) {
    selectedAnnotation_->clearCenterPoints();
    redrawCurrentSelections();
  }
}

void AnnotationWidget::handleDragged(int x, int y, Qt::MouseButton) {
  if(selectedAnnotation_) {
    auto next = QPoint(x,y) + offset_;
    if(next != previous_) {
      selectedAnnotation_->setCenterPoint(next.x(), next.y(), currentFrame_);
      redrawCurrentSelections();
    }
    previous_ = next;
  }
}

void AnnotationWidget::handleHovered(int x, int y) {
  // For each annotation a and each selection s:
  //  1. Select s and a if we're hovering over s
  //  2. Set s->hovered() if we're hovering over s
  //  3. Deselect s if we hover over something else
  //  4. Deselect a if we hover over another annotation's selections
  annotationList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  selectionList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  bool updateSelected = false;
  for(auto kvp : annotations_) {
    auto a = kvp.second;
    if(filterAnnotation(a)) continue;
    for(auto s : a->getSelections()) {
      if(s->enclosesPoint(x, y, currentFrame_)) {
        if(selectedAnnotation_ != a) {
          loadSelections(a);
        }
        if(!s->hovered())
          updateSelected = true;
        s->setHovered(true);
      } else s->setHovered(false);
    }
  }
  selecting_ = true;
  if(updateSelected) {
    for(auto kvp : annotations_) {
      bool select = false;
      auto a = kvp.second;
      auto aitem = annotationItems_[a];
      for(auto s : a->getSelections()) {
        auto it = selectionItems_.find(s);
        if(it == selectionItems_.end()) continue;
        if(s->hovered()) {
          select = true;
          aitem->setSelected(true);
          it->second->setSelected(true);
          break;
        } else {
          it->second->setSelected(false);
        }
      }
      if(select) break;
      else aitem->setSelected(false);
    }
  }
  selecting_ = false;
  redrawCurrentSelections();
}

void AnnotationWidget::showEvent(QShowEvent* event) {
}

void AnnotationWidget::hideEvent(QHideEvent* event) {
  emit setCurrentSelections({});
  cbxEnable->setChecked(false);
}

void AnnotationWidget::updateFrameSelection() {
  if(rdoAll->isChecked()) {
    minBox->setValue(0);
    maxBox->setValue(maxFrames_ - 1);
    minBox->setEnabled(false);
    maxBox->setEnabled(false);
  } else if(rdoCurrent->isChecked()) {
    minBox->setValue(currentFrame_);
    maxBox->setValue(currentFrame_);
    minBox->setEnabled(false);
    maxBox->setEnabled(false);
  } else {
    minBox->setEnabled(true);
    maxBox->setEnabled(true);
  }
}

void AnnotationWidget::loadConfig(const ToolConfig& config) {
  config_ = config.annotationConfig;
  cameraBox->setCurrentIndex(static_cast<int>(config_.camera));
  colorBox->setCurrentIndex(static_cast<int>(config_.color));
  selectionTypeBox->setCurrentIndex(static_cast<int>(config_.selection_type));
  cbxAuto->setChecked(config_.auto_create);
}

void AnnotationWidget::saveConfig(ToolConfig& config) {
  config.annotationConfig = config_;
}

void AnnotationWidget::controlsChanged() {
  if(loading_) return;
  config_.camera = static_cast<Camera::Type>(cameraBox->currentIndex());
  config_.color = static_cast<Color>(colorBox->currentIndex());
  config_.selection_type = static_cast<SelectionType>(selectionTypeBox->currentIndex());
  config_.auto_create = cbxAuto->isChecked();
  ConfigWidget::saveConfig();
}
