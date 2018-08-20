#pragma once

#include <QMainWindow>
class ToolConfig;

class ConfigWindow : public QMainWindow {
Q_OBJECT
  public:
    ConfigWindow(QMainWindow* parent);
  public slots:
    void baseLoadConfig(const ToolConfig& config);
    virtual void loadConfig(const ToolConfig& config) = 0;
    virtual void saveConfig(ToolConfig& config) { }
    void controlsChanged(const QString&) { controlsChanged(); }
    void controlsChanged(double) { controlsChanged(); }
    void controlsChanged(float) { controlsChanged(); }
    void controlsChanged(int) { controlsChanged(); }
    void controlsChanged(short) { controlsChanged(); }
    void controlsChanged(char) { controlsChanged(); }
    void controlsChanged(bool) { controlsChanged(); }
    virtual void controlsChanged() { }
  protected:
    bool loading_;
    void saveConfig();
};

