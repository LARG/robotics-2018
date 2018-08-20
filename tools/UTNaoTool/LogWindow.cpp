#include <QtGui>
#include <iostream>
#include <tool/UTMainWnd.h>
#include <tool/LogWindow.h>

using namespace std;

LogWindow::LogWindow(QMainWindow* p) : ConfigWindow(p) {
  setupUi(this);
  setWindowTitle(tr("Log Window"));
  prevFrame=-1;
  parent = p;
  currFrame = 0;

  logLevelHighBox->setValue(40);
  logLevelLowBox->setValue(0);

  connect (logLevelHighBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)) );
  connect (logLevelLowBox, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)) );
  connect (moduleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(moduleChanged(int)));
  connect (searchString, SIGNAL(editingFinished()), this, SLOT(updateSearchString()));

  // must be in order from TextLogger.h
  moduleBox->addItem("Vision");
  moduleBox->addItem("Behavior");
  moduleBox->addItem("Localization");
  moduleBox->addItem("Opponent Tracking");
  moduleBox->addItem("Kinematics");
  moduleBox->addItem("Sensors");
  moduleBox->addItem("All");

  moduleType = LoggingModuleMethods::index(LoggingModule::Behavior);
  moduleBox->setCurrentIndex(moduleType);

  connect(moduleBox, SIGNAL(currentIndexChanged(int)), (UTMainWnd*)parent, SLOT(saveConfig(int)));
  connect(logLevelHighBox, SIGNAL(valueChanged(int)), (UTMainWnd*)parent, SLOT(saveConfig(int)));
  connect(logLevelLowBox, SIGNAL(valueChanged(int)), (UTMainWnd*)parent, SLOT(saveConfig(int)));
}

// load the text file
void LogWindow::loadTextFile(const char* path){
  cout << "Opening Text Log  " << path << flush;

  ifstream textFile(path);
  textEntries.clear();

  if (not textFile.good()) {
    cout << "  -  ** ERROR OPENING FILE ***\n";
    return;
  }

  std::string line;
  while(std::getline(textFile, line))
    textEntries.push_back(line);

  cout << "  - Done with " << textEntries.size() << " lines!" << "\n";
  updateFrame(NULL);
}

void LogWindow::loadConfig(const ToolConfig& config) {
  logLevelLowBox->setValue(config.logLevelLow);
  logLevelHighBox->setValue(config.logLevelHigh);
  moduleBox->setCurrentIndex(config.moduleTypeIndex);
}

void LogWindow::saveConfig(ToolConfig& config) {
  config.logLevelLow = logLevelLowBox->value();
  config.logLevelHigh = logLevelHighBox->value();
  config.moduleTypeIndex = moduleType;
}

void LogWindow::moduleChanged(int n){
  moduleType = n;
  prevFrame = -1;
  updateFrame(NULL);
}

void LogWindow::updateLevel(int /*n*/){
  prevFrame = -1;
  updateFrame(NULL);
}

void LogWindow::updateSearchString(){
  prevFrame = -1;
  updateFrame(NULL);
}


void LogWindow::setText(std::vector<std::string> text){
  textEntries = text;
}

// will fix this at some point so its more efficient and doesnt search
// the whole vector every time.
void LogWindow::updateFrame(MemoryFrame* mem) {
  if (textEntries.size() == 0){
    updateMemoryBlocks(mem);
    return;
  }
  if (mem != NULL){
    // get frame #
    FrameInfoBlock* frameInfo = NULL;
    mem->getBlockByName(frameInfo, "vision_frame_info",true);
    if (frameInfo != NULL) currFrame = frameInfo->frame_id;
  }

  //if (currFrame==prevFrame) return;  // Don't do anything if frame is the same

  prevFrame=currFrame;
  QString text;
  QString line;
  int frame;
  int loglev;
  int module;
  int lowLevel = logLevelLowBox->value();
  int highLevel = logLevelHighBox->value();
  QString sep = ":";
  QString search = searchString->text();
  for (int i = 0; i < (int)textEntries.size(); i++) {
    sscanf(textEntries[i].c_str(),"loglev %i: frame %i: module %i",
           &loglev,&frame,&module);

    //if (frame > currFrame) break; // quit loop //commented this line out because frames may be non-sequential for combined logs

    //cout << logManager->textEntries[i].c_str() << endl << flush;

    // dont display stuff over loglevel
    if (loglev < lowLevel || loglev > highLevel)
      continue;

    // dont display stuff from other modules
    if (moduleType != LoggingModuleMethods::NumLoggingModules() && moduleType != module)
      continue;

    if (frame == currFrame) {
      // separate out stuff after loglevel and frame #
      line = textEntries[i].c_str();

      // possibly check for search string
      if (search.size() > 0){
        if (line.indexOf(search) == -1)
          continue;
      }

      // find 3nd colon, allow later colons
      int firstColon = line.indexOf(sep, 0);
      int secondColon = line.indexOf(sep, firstColon+1);
      int thirdColon = line.indexOf(sep, secondColon+1);

      QString stuff = line.remove(0, thirdColon+1);
      text += stuff + "\n";

    }
  }
  textBrowser->setPlainText(text);

  updateMemoryBlocks(mem);
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

