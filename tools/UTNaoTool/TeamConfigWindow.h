#ifndef TEAMCONFIG_WINDOW_H
#define TEAMCONFIG_WINDOW_H

#include <QWidget>
#include <common/TeamConfig.h>
#include "ProcessExecutor.h"
#include "ui_TeamConfigWindow.h"

class QLabel;
class QWidget;
class QFile;

class TeamConfigWindow : public QMainWindow, public Ui_TeamConfigWindow {
 Q_OBJECT

  public:
  TeamConfigWindow(QMainWindow* par);
  QString getFullIP(const QString &suffix);
  RobotConfig getRobotConfig(int self);
  
  FILE* localConfig;
  
  QStringList getUploadList();
  QMainWindow* parent;

  signals:
    void robotStatusUpdated(int robot, ProcessExecutor::RobotStatus status);
    void logStatusUpdated(int robot, QString status);

  public slots:
    void updateRobotStatus(int robot, ProcessExecutor::RobotStatus status);
    void updateLogStatus(int robot, QString status);
    void clearRobotStatus(int robot);
    void updateSSH();
    void reloadLocalConfig();
    void saveLocalConfig();
    void startNaoQi();  
    void stopNaoQi();
    void restartInterpreter();  
    void uploadEverything();
    void uploadBinary();
    void uploadRobotConfig();
    void uploadMotionVisionConfig();
    void uploadColor();
    void uploadWireless();
    void uploadInterpreter();
    void checkStatus();
    void uploadTime();

    void startLogging();
    void stopLogging();
    void copyLogs();
    void deleteLogs();
  private:
    std::map<int, QCheckBox*> checks_;
    std::map<int, QLabel*> statuses_;
    std::map<int, QLineEdit*> ipboxes_;
    std::map<int, QSpinBox*> posX_, posY_, posT_;
    std::map<int, QLabel*> logStatuses_;
    std::vector<int> robots_;
    ProcessExecutor executor_;
};


#endif
