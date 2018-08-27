#include <QtGui>
#include <iostream>
#include <tool/UTMainWnd.h>
#include <tool/LogWindow.h>

using namespace std;

LogWindow::LogWindow(QMainWindow* p) : ConfigWindow(p) {
  setupUi(this);
  setWindowTitle(tr("Text Log Window"));
  parent = p;
  currFrame = 0;

  logLevelHighBox->setValue(40);
  logLevelLowBox->setValue(0);

  connect (logLevelHighBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)) );
  connect (logLevelLowBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)) );
  connect (moduleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(moduleChanged(int)));
  connect (searchString, SIGNAL(editingFinished()), this, SLOT(updateSearchString()));

  // must be in order from TextLogger.h
  for(const auto& name : LoggingModuleMethods::names())
    moduleBox->addItem(name.c_str());
  moduleBox->addItem("All");

  moduleBox->setCurrentIndex(static_cast<int>(TextLogger::Type::Behavior));

  connect(moduleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(controlsChanged(int)));
  connect(logLevelHighBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
  connect(logLevelLowBox, SIGNAL(valueChanged(int)), this, SLOT(controlsChanged(int)));
}

void LogWindow::showEvent(QShowEvent* event) {
  if(!path_.empty())
    loadTextFile(path_);
}

void LogWindow::insert(LogEntry&& entry) {
  if(entries_.find(entry.frame) == entries_.end())
    entries_[entry.frame] = unordered_map<int,vector<LogEntry>>();
  if(entries_[entry.frame].find(entry.module) == entries_[entry.frame].end())
    entries_[entry.frame][entry.module] = vector<LogEntry>();
  entries_[entry.frame][entry.module].push_back(entry);
}

// load the text file
void LogWindow::loadTextFile(std::string path) {
  if(isVisible()) {
    path_ = "";
  } else {
    path_ = path;
    return;
  }

  ifstream infile(path);
  if (!infile.good()) {
    return;
  }
  std::string line;
  std::vector<std::string> lines;
  while(std::getline(infile, line))
    lines.push_back(line);
  processLines(std::move(lines));
  updateFrame(NULL);
}

void LogWindow::processLines(vector<string>&& lines) {
  entries_.clear();
  LogEntry current;
  thread_local std::array<char,4096> buffer;
  for(const auto& line : lines) {
    buffer.fill(0);
    LogEntry next;
    int read = sscanf(line.c_str(), "loglev %i: frame %i: module %i:%4095c", 
      &next.level, &next.frame, &next.module, buffer.data()
    );
    if(read == 4) {
      if(current.module >= 0)
        insert(std::move(current));
      current = next;
      current.text = buffer.data();
      current.text += "\n";
    } else {
      current.text += line + "\n";
    }
  }
  if(current.module >= 0)
    insert(std::move(current));
}

void LogWindow::loadConfig(const ToolConfig& tconfig) {
  config_ = tconfig.logWindowConfig;
  logLevelLowBox->setValue(config_.minLevel);
  logLevelHighBox->setValue(config_.maxLevel);
  moduleBox->setCurrentIndex(config_.module);
  filterBox->setChecked(config_.filterTextLogs);
}

void LogWindow::saveConfig(ToolConfig& tconfig) {
  tconfig.logWindowConfig = config_;
}

void LogWindow::controlsChanged() {
  if(loading_) return;
  config_.minLevel = logLevelLowBox->value();
  config_.maxLevel = logLevelHighBox->value();
  config_.module = moduleBox->currentIndex();
  config_.filterTextLogs = filterBox->isChecked();
  ConfigWindow::saveConfig();
}

void LogWindow::moduleChanged(int n){
  updateFrame(NULL);
}

void LogWindow::updateLevel(int /*n*/){
  updateFrame(NULL);
}

void LogWindow::updateSearchString(){
  updateFrame(NULL);
}


void LogWindow::setText(std::vector<std::string> text) {
  processLines(std::move(text));
}

void LogWindow::updateFrame(MemoryFrame* mem, bool append) {
  if (entries_.size() == 0){
    updateMemoryBlocks(mem);
    return;
  }

  if (mem != NULL){
    FrameInfoBlock* frameInfo = NULL;
    mem->getBlockByName(frameInfo, "vision_frame_info",true);
    if (frameInfo != NULL) currFrame = frameInfo->frame_id;
  }

  QString text;
  auto entries = entries_[currFrame][config_.module];
  auto ss = searchString->text().toStdString();
  int lowLevel = logLevelLowBox->value();
  int highLevel = logLevelHighBox->value();
  for(const auto& entry : entries) {
    if(entry.level < lowLevel || entry.level > highLevel) continue;
    if(ss.size() > 0) {
      if(entry.text.find(ss) == std::string::npos)
        continue;
    }
    text += entry.text.c_str();
  }
  if(append)
    textBrowser->append(text);
  else
    textBrowser->setPlainText(text);
  updateMemoryBlocks(mem);
  update();
}

void LogWindow::updateMemoryBlocks(MemoryFrame *mem) {
  QString text;
  std::vector<std::string> block_names;

  if (mem != NULL) {
    mem->getBlockNames(block_names,false);
    for (unsigned int i = 0; i < block_names.size(); i++)
      text += QString::fromStdString(block_names[i] + '\n');
  }

  memoryBlocks->setPlainText(text);
}


void LogWindow::keyPressEvent(QKeyEvent *event) {
  //bool ctrl = event->modifiers() & (Qt::ControlModifier);
  switch (event->key()) {
  case Qt::Key_Comma:
    emit prevSnapshot();
    break;
  case Qt::Key_Period:
    emit nextSnapshot();
    break;
  }
}

