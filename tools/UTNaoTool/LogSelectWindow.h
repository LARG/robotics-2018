#ifndef LOGSELECT_WINDOW_H
#define LOGSELECT_WINDOW_H

#include <communications/UDPWrapper.h>
#include <communications/CommInfo.h>

#include <QMainWindow>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <QLabel>

#include <functional>

#include <tool/ConfigWindow.h>

class LogSelectWindow : public ConfigWindow {
  Q_OBJECT

  private:
    static LogSelectWindow* instance_;
    bool log_enabled_;
    static void listenUDP(void*);
    void listenForLoggingStatus();
    UDPWrapper* naoUDP;
    void logModeOn();

  public:
    LogSelectWindow(QMainWindow* pa,std::vector<std::string> &block_names);
    static LogSelectWindow* inst();

    QMainWindow* parent;

    QLabel** moduleLabels;
    QCheckBox** moduleChecks;
    int NUM_MODULES;

    QWidget* centralWidget;
    QPushButton* logButton;
    QPushButton* forceStopLogButton;
    QCheckBox* batchButton;
    QPushButton* sendButton;
    QSpinBox* frameCount;
    QDoubleSpinBox* frequency;

    QLabel** groupLabels;
    QCheckBox** groupChecks;

    std::vector<std::string> block_names_;


   public slots:
     void logModeOff(bool force = true);
     void sendLogSettings();
     void sendLogSettings(QString ip);
     void toggleLogEnabled();
     void requestStopLog();

     void locGroupToggled(bool toggle);
     void visionGroupToggled(bool toggle);
     void visionRawGroupToggled(bool toggle);
     void behaviorGroupToggled(bool toggle);
     void allGroupToggled(bool toggle);

     void updateSelectedIP(QString address);

     void loadConfig(const ToolConfig& config);
     void saveConfig(ToolConfig& config);

     void startMultiLogging(std::vector<QString> ips);
     void stopMultiLogging(std::vector<QString> ips, std::function<void(QString)> callback);
};

#endif
