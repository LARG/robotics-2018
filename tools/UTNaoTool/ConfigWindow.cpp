#include <tool/ConfigWindow.h>
#include <tool/UTMainWnd.h>

ConfigWindow::ConfigWindow(QMainWindow* parent) : QMainWindow(parent) {
  connect(UTMainWnd::inst(), SIGNAL(loadingConfig(const ToolConfig&)), this, SLOT(baseLoadConfig(const ToolConfig&)));
  connect(UTMainWnd::inst(), SIGNAL(savingConfig(ToolConfig&)), this, SLOT(saveConfig(ToolConfig&)));
}

void ConfigWindow::saveConfig() {
  UTMainWnd::inst()->saveConfig();
}

void ConfigWindow::baseLoadConfig(const ToolConfig& config) {
  loading_ = true;
  loadConfig(config);
  loading_ = false;
}
