#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <vector>
#include <string>

#include <memory/MemoryFrame.h>
#include <memory/FrameInfoBlock.h>
#include <memory/TextLogger.h>

#include <tool/ConfigWindow.h>
#include "ui_LogWindow.h"

class LogWindow : public ConfigWindow, public Ui_UTLogWindow {
 Q_OBJECT

  public:
    LogWindow(QMainWindow* pa);

    void updateFrame(MemoryFrame* mem);
    void updateMemoryBlocks(MemoryFrame *mem);

    void loadTextFile(const char* filename);
    std::vector<std::string> textEntries;
    QMainWindow* parent;
    int prevFrame;
    int currFrame;
    
    void setText(std::vector<std::string> text);

    int moduleType;

     
  protected:
    void keyPressEvent(QKeyEvent *event);

  signals:
    void prevSnapshot();
    void nextSnapshot();

 public slots:
    void updateLevel(int n);
    void updateSearchString();
    void moduleChanged(int n);
    void loadConfig(const ToolConfig& config);
    void saveConfig(ToolConfig& config);
};

#endif

