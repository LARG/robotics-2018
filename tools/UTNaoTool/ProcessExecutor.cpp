#include "ProcessExecutor.h"
#include <QtCore>

using namespace std;
typedef ProcessExecutor PE;

ToolProcess::ToolProcess() : process(NULL) { }
ToolProcess::~ToolProcess() { if(process) delete process; }

bool PE::workerInitialized_ = false;
mutex PE::qmutex_;
condition_variable PE::qcv_;
vector<queue<ToolProcess>> PE::pqueues_ = vector<queue<ToolProcess>>(PE::NumPriorities);
map<QString,ToolProcess*> PE::connections_;
unique_ptr<thread> PE::worker_thread_;
bool PE::stopping_ = false;

PE::ProcessExecutor() {
  environment_ = QProcessEnvironment::systemEnvironment();
  environment_.remove("PYTHONHOME");
  environment_.remove("PYTHONPATH");
  basepath_ = environment_.value("NAO_HOME");
  logpath_ = basepath_ + "/logs";
  datapath_ = basepath_ + "/data";
  if(!workerInitialized_) {
    qRegisterMetaType<ProcessExecutor::RobotStatus>("ProcessExecutor::RobotStatus");
    workerInitialized_ = true;
    worker_thread_ = std::make_unique<std::thread>(&PE::workerProcess);
  }
}

PE::~ProcessExecutor() {
  stopping_ = true;
  qcv_.notify_one();
  worker_thread_->join();
  worker_thread_.reset();
}

void PE::sendCopyRobotCommand(QString ip, QString command, bool verbose, PE::Callback callback) {
  auto func = [=] () { 
    sendCopyRobotCommandSync(ip, command, verbose, callback);
  };
  queueProcess(func, "copy robot: " + command);
}

bool PE::sendCopyRobotCommandSync(QString ip, QString command, bool verbose, PE::Callback callback) {
  QProcess copy_robot;
  copy_robot.setProcessEnvironment(environment_);

  QString cmd(basepath_);
  cmd += ("/bin/copy_robot ");
  cmd += ip + " ";
  cmd += command;
  //if (verbose)
    cout << "Executing: " << cmd.toStdString() << endl;

  copy_robot.start(cmd);
  if (!copy_robot.waitForStarted()) {
    cout << "Unable to launch copy robot script" << endl << flush;
    return false;
  }
  copy_robot.closeWriteChannel();
  copy_robot.waitForFinished(-1);

  if (verbose) { // print stdout and stderr as well
    QByteArray std_out = copy_robot.readAllStandardOutput();
    QByteArray std_err = copy_robot.readAllStandardError();
    QString outStr(std_out), errStr(std_err);
    cout << outStr.toStdString();
    cout << errStr.toStdString();
  } else { // only print stderr
    QByteArray err_result = copy_robot.readAllStandardError();
    QString resultStr(err_result);
    cout << resultStr.toStdString();
  }

  // TODO: the exist status always seems 0, hence this does not really work
  bool result = copy_robot.exitStatus() == QProcess::NormalExit;
  callback(result);
  return true;
}

void PE::openSSH(QString ip, StatusCallback callback) {
  if(connections_.find(ip) != connections_.end()) {
    checkRobotStatus(ip, callback);
    return;
  }
  ToolProcess* tp = new ToolProcess();
  QString name = "Open SSH: ";
  callback(Connecting);
  tp->task = [=] {
    QProcess* scp = tp->process = new QProcess();
    QString host = "nao@";
    host.append(ip);
    QStringList args;
    args.push_back(host);
    args.push_back("-tt");
    QString cmd = "ssh";
    scp->setProcessChannelMode(QProcess::MergedChannels);
    scp->start(cmd, args);
    if(scp->waitForStarted()) {
      if(scp->waitForReadyRead()) {
        connections_[ip] = tp;
        QString s = scp->readAllStandardOutput();
        if(!s.contains("No route to host")) {
          checkRobotStatus(ip, callback);
          return;
        }
      }
    }
    tp->aborted = true;
    callback(Dead);
  };
  tp->name = name + ip;
  queueProcess(*tp, High);
}

void PE::closeSSH(QString ip, StatusCallback callback) {
  if(connections_.find(ip) != connections_.end()) {
    auto tp = connections_[ip];
    connections_.erase(ip);
    delete tp;
  }
  checkRobotStatus(ip, callback);
}

void PE::stopNaoqi(QString ip) {
  sendNaoqiCommand(ip, "stop");
}

void PE::startNaoqi(QString ip) {
  sendNaoqiCommand(ip, "start");
}

void PE::restartNaoqi(QString ip) {
  sendNaoqiCommand(ip, "restart");
}

void PE::sendNaoqiCommand(QString ip, QString command) {
  auto func = [=] {
    cout << (command + " NaoQi on " + ip).toStdString() << endl;
    QProcess scp;
    QString out = "nao@";
    out.append(ip);
    QStringList args;
    args.push_back(out);
    args.push_back("sudo /etc/init.d/naoqi " + command + " &> /dev/null");
    QString cmd = "ssh";
    scp.start(cmd, args);
    if (!scp.waitForStarted()) {
      cout << (command + " Error 1").toStdString() << endl << flush;
      return;
    }
    scp.waitForFinished(120000);
    scp.closeWriteChannel();
    cout << "done!\n";
  };
  queueProcess(func, command + " naoqi", High);
}

void PE::compile(PE::Callback callback) {
  auto func = [=] {
    cout << "Compiling with optimizations enabled...";
    std::cout.flush();
    QString command = basepath_ + "/build/compile";
    QStringList args;
    args.push_back("robot");
    args.push_back("--optimize");
    executeSimpleCommand(command, args, true);
    cout << "done!";
  };
  queueProcess(func, "compile for robot");
}

void PE::setRobotTime(QString ip) {
  auto func = [=] {
    cout << "Updating robot time...";
    QString command = basepath_;
    command += "/install/setup_robot";
    QStringList args;
    args.push_back("--date");
    args.push_back("--robotIP");
    args.push_back(ip);
    executeSimpleCommand(command, args);
    cout << "done!\n";
  };
  queueProcess(func, "set robot time");
}

void PE::getLogs(QString ip, PE::Callback callback) {
  getLogs(ip, logpath_, callback);
}

void PE::getLogs(QString ip, QString destination, PE::Callback callback) {
  auto getlogs = [=] {
    cout << "Copying logs from " << ip.toStdString() << "\n";

    QString out = "nao@";
    out.append(ip);
    out.append(":~/logs/vision*");

    QStringList args;
    args.push_back("-avz");
    args.push_back(out);
    args.push_back(destination);

    executeSimpleCommand("rsync", args, true);
    cout << "Done!" << endl;
    callback(true);
  };
  queueProcess(getlogs, "get logs");
}

void PE::removeLogs(QString ip, PE::Callback callback) {
  auto func = [=] {
    QString command = "ssh";
    QString out = "nao@";
    out.append(ip);

    QStringList args;
    args.push_back(out);
    args.push_back("rm -rf ~/logs/vision*");
    cout << "Removing logs from " << ip.toStdString() << "...";
    executeSimpleCommand(command, args);
    cout << "done!\n";
    callback(true);
  };
  queueProcess(func, "remove logs");
}

bool PE::executeSimpleCommand(QString command, bool verbose) {
  QStringList args;
  return executeSimpleCommand(command, args, verbose);
}

bool PE::executeSimpleCommand(QString command, QStringList args, bool verbose) {
  QProcess process;
  process.setProcessEnvironment(environment_);
  if(verbose) process.setProcessChannelMode(QProcess::ForwardedChannels);
  process.start(command, args);
  if(!process.waitForStarted()) return false;
  process.waitForFinished(-1);
  return true;
}

void PE::checkRobotStatus(QString ip, StatusCallback callback) {
  if(connections_.find(ip) != connections_.end()) {
    if(connections_[ip]->aborted) {
      callback(Dead); return;
    }
    QString home(getenv("HOME"));
    QDir qdir(home + "/.ssh");
    auto qelist = qdir.entryList(QStringList("*nao@" + ip + "*"), QDir::System);
    if(qelist.size() > 0)
      callback(Connected);
    else
      callback(Connecting);
    return;
  }
  auto aliveCheck = [=] {
    QProcess fping;
    QStringList cmd;
    cmd.push_back("-r1 -t100");
    cmd.push_back(ip);
    
    fping.start("fping", cmd);
    
    if (!fping.waitForStarted()) {
      cout << "fping Error 1 - Try installing fping, if that doesn't work then comment this out" << endl << flush;
      return;
    }
    
    fping.closeWriteChannel();
    fping.waitForFinished();
    
    QByteArray result = fping.readAll();
    QString s_data = QString::fromAscii(result.data());
    if(result.contains("alive"))
      callback(Alive);
    else
      callback(Dead);
  };
  queueProcess(aliveCheck, "check robot status");
}

void PE::sendRobotConfig(QString ip, RobotConfig config, bool verbose) {
  auto func = [=] {
    QString path = basepath_ + "/build/config.yaml";
    config.saveToFile(path.toStdString());
    sendCopyRobotCommandSync(ip, "simple_config --config_file \"" + path + "\"", verbose);
  };
  queueProcess(func, "send config");
}

void PE::verifyRobotConfig(QString ip, RobotConfig config, bool verbose) {
  auto func = [=] {
    QString path = basepath_ + "/build/config.yaml";
    config.saveToFile(path.toStdString());
    sendCopyRobotCommandSync(ip, "simple_config --config_file \"" + path + "\" --verify", verbose);
  };
  queueProcess(func, "verify config");
}
void PE::sendPython(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "python", verbose);
}

void PE::verifyPython(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "python --verify", verbose);
}

void PE::sendBinary(QString ip, bool verbose, bool optimize) {
  if(optimize)
    sendCopyRobotCommand(ip, "nao motion vision --optimize", verbose);
  else
    sendCopyRobotCommand(ip, "nao motion vision", verbose);
}

void PE::verifyBinary(QString ip, bool verbose, bool optimize) {
  if(optimize)
    sendCopyRobotCommand(ip, "nao motion vision --verify --optimize", verbose);
  else
    sendCopyRobotCommand(ip, "nao motion vision --verify", verbose);
}

void PE::sendConfigFiles(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "configs", verbose);
  sendCopyRobotCommand(ip, "scripts", verbose);
}
void PE::verifyConfigFiles(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "configs --verify", verbose);
  sendCopyRobotCommand(ip, "scripts --verify", verbose);
}


void PE::sendMotionFiles(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "motion_file", verbose);
}

void PE::verifyMotionFiles(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "motion_file --verify", verbose);
}

void PE::sendColorTable(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "color_table", verbose);
}

void PE::verifyColorTable(QString ip, bool verbose) {
  sendCopyRobotCommand(ip, "color_table --verify", verbose);
}

void PE::notifyCompletion(PE::Callback callback) {
  queueProcess([=]{ callback(true); }, "notify completion");
}

void PE::queueProcess(PE::Task task, const QString& name, ProcessPriority priority) {
  ToolProcess tp;
  tp.task = task;
  tp.name = name;
  queueProcess(tp, priority);
}

void PE::queueProcess(ToolProcess tp, ProcessPriority priority) {
  std::lock_guard<std::mutex> lock(qmutex_);
  auto& pqueue = pqueues_[priority];
  pqueue.push(tp);
  qcv_.notify_one();
}

void PE::workerProcess() {
  while(!stopping_) {
    {
      std::unique_lock<std::mutex> lock(qmutex_);
      qcv_.wait(lock, []{
        if(stopping_) return true;
        for(auto& pq : pqueues_)
          if(!pq.empty()) return true;
        return false;
      });
    }
    for(auto& pq : pqueues_) {
      while(!pq.empty()) {
        ToolProcess tp;
        {
          std::lock_guard<std::mutex> lock(qmutex_);
          tp = pq.front();
          pq.pop();
        }
        tp.task();
      }
    }
  }
}
