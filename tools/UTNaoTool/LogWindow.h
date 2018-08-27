#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <vector>
#include <string>
#include <unordered_map>

#include <memory/MemoryFrame.h>
#include <memory/FrameInfoBlock.h>
#include <memory/TextLogger.h>

#include <tool/ConfigWindow.h>
#include <tool/LogWindowConfig.h>
#include "ui_LogWindow.h"

class LogWindow : public ConfigWindow, public Ui_UTLogWindow {
 Q_OBJECT

  public:
    LogWindow(QMainWindow* pa);

    void updateFrame(MemoryFrame* mem, bool append=false);
    void updateMemoryBlocks(MemoryFrame *mem);

    void loadTextFile(std::string path);
    std::vector<std::string> textEntries;
    QMainWindow* parent;
    int currFrame;
    
    void setText(std::vector<std::string> text);

    struct LogEntry {
      int module = -1;
      int frame = -1;
      int level = -1;
      std::string text = "";
    };

  protected:
    void processLines(std::vector<std::string>&& lines);
    void insert(LogEntry&& entry);
    std::unordered_map<int,std::unordered_map<int,std::vector<LogEntry>>> entries_;
    LogWindowConfig config_;
    std::string path_;
     
  protected:
    void keyPressEvent(QKeyEvent *event);

  signals:
    void prevSnapshot();
    void nextSnapshot();

  private slots:
    void controlsChanged() override;

  protected slots:
    void showEvent(QShowEvent* event) override;
    
  public slots:
    void updateLevel(int n);
    void updateSearchString();
    void moduleChanged(int n);
    void loadConfig(const ToolConfig& tconfig) override;
    void saveConfig(ToolConfig& tconfig) override;
};

#endif

