#pragma once

#include "ui_MotionWindow.h"
#include <tool/ConfigWindow.h>
#include <memory/MemoryCache.h>

class AnnotationGroup;

class MotionWindow : public ConfigWindow, public Ui_MotionWindow {
 Q_OBJECT

  public:
    MotionWindow(QMainWindow* pa);
    void updateMemory(MemoryFrame* mem);
    void loadConfig(const ToolConfig& config) { }
    void saveConfig(ToolConfig& config) { }
  signals:
    void prevSnapshot();
    void nextSnapshot();
  private:
    MemoryCache cache_;
};
