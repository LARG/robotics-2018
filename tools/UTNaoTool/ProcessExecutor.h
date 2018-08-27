#pragma once

#include <iostream>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

#include <QProcess>

#include <common/RobotConfig.h>

#define CALLBACK(callback) Callback callback = [](bool){}
#define STATUS_CALLBACK(callback) StatusCallback callback = [](RobotStatus){}

struct ToolProcess {
  std::function<void()> task;
  QString name;
  QProcess* process;
  ToolProcess();
  ~ToolProcess();
  bool aborted = false;
};

class ProcessExecutor {
  public:
    enum ProcessPriority {
      High,
      Normal,
      NumPriorities
    };
    enum RobotStatus {
      Dead,
      Alive,
      Connecting,
      Connected
    };
      
    ProcessExecutor();
    ~ProcessExecutor();
    typedef std::function<void(bool)> Callback;
    typedef std::function<void(RobotStatus)> StatusCallback;
    typedef std::function<void()> Task;
    void sendCopyRobotCommand(QString command, QString ip, bool verbose, CALLBACK(callback));
    void openSSH(QString ip, STATUS_CALLBACK(callback));
    void closeSSH(QString ip, STATUS_CALLBACK(callback));
    void stopNaoqi(QString ip);
    void startNaoqi(QString ip);
    void restartNaoqi(QString ip);
    void sendNaoqiCommand(QString ip, QString command);
    void setRobotTime(QString ip);
    void getLogs(QString ip, CALLBACK(callback));
    void getLogs(QString ip, QString destination, CALLBACK(callback));
    void removeLogs(QString ip, CALLBACK(callback));
    void checkRobotStatus(QString ip, STATUS_CALLBACK(callback));

    void sendRobotConfig(QString ip, RobotConfig config, bool verbose = true);
    void verifyRobotConfig(QString ip, RobotConfig config, bool verbose = true);

    void sendMotionFiles(QString ip, bool verbose = true);
    void verifyMotionFiles(QString ip, bool verbose = true);

    void sendConfigFiles(QString ip, bool verbose = true);
    void verifyConfigFiles(QString ip, bool verbose = true);

    void sendWireless(QString ip, bool verbose = true);

    void sendPython(QString ip, bool verbose = true);
    void verifyPython(QString ip, bool verbose = true);

    void sendColorTable(QString ip, bool verbose = true);
    void verifyColorTable(QString ip, bool verbose = true);

    void sendBinary(QString ip, bool verbose = true, bool optimize = false);
    void verifyBinary(QString ip, bool verbose = true, bool optimize = false);

    void notifyCompletion(CALLBACK(callback));

    void compile(CALLBACK(callback));

  private:
    bool sendCopyRobotCommandSync(QString command, QString ip, bool verbose, CALLBACK(callback));
    bool executeSimpleCommand(QString command, bool verbose = false);
    bool executeSimpleCommand(QString command, QStringList args, bool verbose = false);
    QProcessEnvironment environment_;
    QString basepath_, logpath_, datapath_;

    static std::map<QString,ToolProcess*> connections_;
    static std::mutex qmutex_;
    static std::condition_variable qcv_;
    static std::vector<std::queue<ToolProcess>> pqueues_;
    static std::unique_ptr<std::thread> worker_thread_;
    static void queueProcess(Task task, const QString& name, ProcessPriority priority = Normal);
    static void queueProcess(ToolProcess tp, ProcessPriority priority = Normal);
    static void workerProcess();
    static bool workerInitialized_;
    static bool stopping_;
};
