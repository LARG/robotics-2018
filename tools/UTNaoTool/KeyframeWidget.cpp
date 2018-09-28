#include <tool/KeyframeWidget.h>
#include <tool/UTMainWnd.h>
#include <common/Util.h>

void KeyframeItem::updateName() {
  auto s = keyframe_.name = txtName->toPlainText().toStdString();
  util::sreplace(s, {"\r","\n"}, "");
  if(s != keyframe_.name) {
    keyframe_.name = s;
    txtName->setPlainText(QString::fromStdString(keyframe_.name));
  }
  lblName->setText(txtName->toPlainText());
}

void KeyframeItem::updateFrames(int frames) {
  keyframe_.frames = frames;
  lblFrames->setText(QString::number(frames) + spnFrames->suffix());
}

void KeyframeItem::activate() {
  lblName->setVisible(false);
  lblFrames->setVisible(false);
  txtName->setVisible(true);
  spnFrames->setVisible(true);
}

void KeyframeItem::deactivate() {
  lblName->setVisible(true);
  lblFrames->setVisible(true);
  txtName->setVisible(false);
  spnFrames->setVisible(false);
}

void KeyframeItem::swap(QListWidgetItem* oitem) {
  auto kfitem = static_cast<KeyframeItem*>(list_->itemWidget(oitem));
  auto kf = kfitem->keyframe();
  kfitem->updateKeyframe(keyframe_);
  updateKeyframe(kf);

  auto s = oitem->isSelected();
  oitem->setSelected(item_->isSelected());
  item_->setSelected(s);
}

void KeyframeItem::moveUp() {
  int r = list_->row(item_);
  if(r > 0) {
    auto other = list_->item(r - 1);
    swap(other);
  }
}

void KeyframeItem::moveDown() {
  int r = list_->row(item_);
  if(r < list_->count() - 1) {
    auto other = list_->item(r + 1);
    swap(other);
  }
}

QListWidgetItem* KeyframeItem::createParentItem(int row) {
  auto item = new QListWidgetItem();
  item->setSizeHint(QSize(100, 55));
  //item->setFlags(item->flags() | Qt::ItemIsEditable);
  if(row < 0) row = list_->count();
  list_->insertItem(row, item);
  list_->setItemWidget(item, this);
  return item;
}

void KeyframeItem::updateKeyframe(const Keyframe& kf) {
  keyframe_ = kf;
  txtName->setPlainText(QString::fromStdString(keyframe_.name));
  spnFrames->setValue(keyframe_.frames);
}
    
void KeyframeItem::init(int row) {
  connect(txtName, SIGNAL(textChanged()), this, SLOT(updateName()));
  connect(spnFrames, SIGNAL(valueChanged(int)), this, SLOT(updateFrames(int)));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));
  updateKeyframe(keyframe_);
  
  list_ = static_cast<QListWidget*>(parent());
  item_ = createParentItem(row);
  
  deactivate();
}

KeyframeWidget::KeyframeWidget(QWidget* parent) : ConfigWidget(parent) {
  setupUi(this);
  connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));
  connect(btnReload, SIGNAL(clicked()), this, SLOT(reload()));
  connect(btnSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));
  connect(btnLoad, SIGNAL(clicked()), this, SLOT(load()));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(addKeyframe()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteKeyframe()));
  connect(btnPlay, SIGNAL(clicked()), this, SLOT(play()));
  connect(btnShow, SIGNAL(clicked()), this, SLOT(show()));
  connect(keyframeBox, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(updateItem(QListWidgetItem*)));
  connect(keyframeBox, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(activate(QListWidgetItem*)));
  connect(keyframeBox, SIGNAL(itemSelectionChanged()), this, SLOT(deactivateCurrent()));
  connect(rdoTorso, SIGNAL(toggled(bool)), this, SLOT(supportBaseUpdated(bool)));
  connect(rdoLeftFoot, SIGNAL(toggled(bool)), this, SLOT(supportBaseUpdated(bool)));
  connect(rdoRightFoot, SIGNAL(toggled(bool)), this, SLOT(supportBaseUpdated(bool)));
  connect(rdoSensor, SIGNAL(toggled(bool)), this, SLOT(supportBaseUpdated(bool)));
  keyframeTimer_ = new QTimer(this);
  connect(keyframeTimer_, SIGNAL(timeout()), this, SLOT(playNextFrame()));
  keyframeTimer_->setSingleShot(true);
  activated_ = NULL;
  kfconfig_.sequence_file = UTMainWnd::dataDirectory() + "/kicks/default.yaml";
  
}

void KeyframeWidget::loadConfig(const ToolConfig& config) {
  kfconfig_ = config.kfConfig;
  switch(kfconfig_.base) {
    case SupportBase::TorsoBase: rdoTorso->setChecked(true); break;
    case SupportBase::SensorFoot: rdoSensor->setChecked(true); break;
    case SupportBase::LeftFoot: rdoLeftFoot->setChecked(true); break;
    case SupportBase::RightFoot: rdoRightFoot->setChecked(true); break;
  }
  emit updatedSupportBase(kfconfig_.base);
  reload();
}

void KeyframeWidget::saveConfig(ToolConfig& config) {
  config.kfConfig = kfconfig_;
}

void KeyframeWidget::updateItem(QListWidgetItem* item) {
  if(auto kitem = dynamic_cast<KeyframeItem*>(keyframeBox->itemWidget(item))) {
    kitem->updateName();
  }
}

void KeyframeWidget::save() {
  KeyframeSequence ks;
  for(int i = 0; i < keyframeBox->count(); i++) {
    auto kitem = static_cast<KeyframeItem*>(keyframeBox->itemWidget(keyframeBox->item(i)));
    ks.keyframes.push_back(kitem->keyframe());
  }
  ks.save(kfconfig_.sequence_file);
  std::cout << "Kick saved to " << kfconfig_.sequence_file << std::endl;

}

void KeyframeWidget::reload() {
  KeyframeSequence ks;
  keyframeBox->clear();
  ks.load(kfconfig_.sequence_file);
  for(auto kf : ks.keyframes) {
    auto kfitem = new KeyframeItem(keyframeBox, kf);
  }
  if(keyframeBox->count()) {
    keyframeBox->item(0)->setSelected(true);
    show();
  }
  if(!loading_)
    ConfigWidget::saveConfig();
  std::cout << "Kick loaded from " << kfconfig_.sequence_file << std::endl;
}

void KeyframeWidget::saveAs() {
  QString file = QFileDialog::getOpenFileName(this, 
    tr("Open Kick File"),
    QString(getenv("NAO_HOME")) + "/data/kicks",
    tr("Kick files (*.yaml)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty())
    return;
  kfconfig_.sequence_file = file.toStdString();
  save();
}

void KeyframeWidget::load() {

  QString file = QFileDialog::getOpenFileName(this, 
    tr("Open Kick File"),
    QString(getenv("NAO_HOME")) + "/data/kicks",
    tr("Kick files (*.yaml)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty())
    return;
  kfconfig_.sequence_file = file.toStdString();
  reload();

}

void KeyframeWidget::addKeyframe() {
  if(cache_.joint == NULL) {
    fprintf(stderr, "No joint block, can't add keyframes.\n");
    return;
  }
  auto kf = Keyframe(util::ssprintf("Keyframe %i", keyframeBox->count()));
  for(int i = 0; i < NUM_JOINTS; i++)
    kf.joints[i] = cache_.joint->values_[i];
  auto kfitem = new KeyframeItem(keyframeBox, kf);
}

void KeyframeWidget::deleteKeyframe() {
  for(auto item : keyframeBox->selectedItems()) {
    auto kfitem = static_cast<KeyframeItem*>(keyframeBox->itemWidget(item));
    if(activated_ == kfitem) activated_ == NULL;
    delete item;
  }
}

void KeyframeWidget::updateMemory(MemoryCache cache) {
  cache_ = cache;
}

void KeyframeWidget::play() {
  currentKeyframe_ = 0;
  currentFrame_ = 0;
  keyframeTimer_->start(100);
}

void KeyframeWidget::show() {
  for(auto item : keyframeBox->selectedItems()) {
    auto kfitem = static_cast<KeyframeItem*>(keyframeBox->itemWidget(item));
    emit showingKeyframe(kfitem->keyframe());
    break;
  }
}

void KeyframeWidget::playNextFrame() {
  // Clear selections from keyframes that have been played
  if(currentKeyframe_ > 0) {
    auto item = keyframeBox->item(currentKeyframe_ - 1);
    item->setSelected(false);
  }

  // Stop if we've played motions between each pair
  if(currentKeyframe_ >= keyframeBox->count() - 1) {
    for(int i = 0; i < keyframeBox->count(); i++)
      keyframeBox->item(i)->setSelected(false);
    return;
  }

  // Pull the first keyframe in the pair
  auto sitem = keyframeBox->item(currentKeyframe_);
  sitem->setSelected(true);
  auto kfstart = static_cast<KeyframeItem*>(keyframeBox->itemWidget(sitem));
  auto& start = kfstart->keyframe();
  
  // Pull the second keyframe in the pair
  auto fitem = keyframeBox->item(currentKeyframe_ + 1);
  fitem->setSelected(true);
  auto kffinish = static_cast<KeyframeItem*>(keyframeBox->itemWidget(fitem));
  auto& finish = kffinish->keyframe();
  
  // If we've played the transition, increment and start on the next pair
  if(currentFrame_ >= finish.frames) {
    currentKeyframe_++;
    currentFrame_ = 0;
    playNextFrame();
    return;
  }

  // Emit the start, end, and # of frames
  emit playingSequence(start, finish, currentFrame_);
  currentFrame_++;
  keyframeTimer_->start(10);
}

void KeyframeWidget::activate(QListWidgetItem* item) {
  if(activated_) activated_->deactivate();
  activated_ = static_cast<KeyframeItem*>(keyframeBox->itemWidget(item));
  activated_->activate();
}

void KeyframeWidget::deactivateCurrent() {
  if(activated_) activated_->deactivate();
  activated_ = NULL;
}

void KeyframeWidget::supportBaseUpdated(bool) {
  if(loading_) return;
  auto base = SupportBase::TorsoBase;
  if(rdoTorso->isChecked()) base = SupportBase::TorsoBase;
  else if(rdoLeftFoot->isChecked()) base = SupportBase::LeftFoot;
  else if(rdoRightFoot->isChecked()) base = SupportBase::RightFoot;
  else if(rdoSensor->isChecked()) base = SupportBase::SensorFoot;
  emit updatedSupportBase(base);
  kfconfig_.base = base;
  ConfigWidget::saveConfig();
}
