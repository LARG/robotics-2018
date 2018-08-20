#ifndef UTMAINWND_H
#define UTMAINWND_H

#include <QtGui/qmainwindow.h>
#include <vector>
#include <stdio.h>

#include <memory/LogViewer.h>
#include <memory/MemoryFrame.h>
#include <memory/LogWriter.h>
#include <communications/StreamingMessage.h>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <communications/CommInfo.h>

using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;


#include <QFileDialog>

#include <VisionCore.h>
#include <common/ToolPacket.h>

#include "ToolConfig.h"
#include "ui_MainWindow.h"

class UTMainWnd;
class FilesWindow;
class LogEditorWindow;
class LogSelectWindow;
class LogWindow;
class MotionWindow;
class PlotWindow;
class VisionWindow;
class WorldWindow;
class JointsWindow;
class WalkWindow;
class StateWindow;
class CameraWindow;
class SensorWindow;
class TeamConfigWindow;
class AnnotationGroup;
class InterpreterModule;

void server(UTMainWnd *main);

class UTMainWnd : public QMainWindow, public Ui_UTNaoTool {
Q_OBJECT
public:
  UTMainWnd(const char *filename = NULL, bool core = false);
  ~UTMainWnd();

  void sendUDPCommand(QString address, ToolPacket packet);
  void sendUDPCommandToCurrent(ToolPacket packet);
  QString getCurrentAddress();

  void setFrameRange(int start = 0, int end = -1) {
    startSpin->setValue(start);
    endSpin->setValue(end);
  }
  static int loadInt(QTextStream &t, bool &ok);
  static UTMainWnd* inst() { return instance_; }
  static VisionCore* core() { return instance_->visionCore_; }
  static InterpreterModule* interpreter() { return instance_->visionCore_->interpreter_; }
  static std::string dataDirectory() { return std::string(getenv("NAO_HOME")) + "/data"; }
  static bool reload_;

private:
  static UTMainWnd* instance_;
  void loadLog(const char *directory);
  void loadLog(std::string directory);

  VisionCore* visionCore_;

  std::unique_ptr<LogViewer> memory_log_;
  bool isStreaming_;
  bool runningCore_;
  bool coreAvailable_;

  MemoryFrame* memory_;
  int current_index_;

  int startFrame;
  int endFrame;

  // streaming
public:
  bool isStreaming() { return isStreaming_; }
  void processStream(socket_ptr sock);
  MemoryFrame stream_memory_;
  StreamingMessage stream_msg_;

  // windows
  FilesWindow* filesWnd_;
  LogEditorWindow* logEditorWnd_;
  LogSelectWindow* logSelectWnd_;
  LogWindow* logWnd_;
  MotionWindow* motionWnd_;
  PlotWindow* plotWnd_;
  VisionWindow* visionWnd_;
  WorldWindow* worldWnd_;
  JointsWindow* jointsWnd_;
  WalkWindow* walkWnd_;
  StateWindow* stateWnd_;
  CameraWindow* cameraWnd_;
  SensorWindow* sensorWnd_;
  TeamConfigWindow* teamWnd_;

  std::string rcPath;

private:
  ToolConfig config_;
  bool loading_;
  LogWriter* logger_;
  AnnotationGroup* annotations_;

public slots:

  void setFrameStep(int);
  void addressChanged();
  void loadConfig();
  void saveConfig(double);
  void saveConfig(int);
  void saveConfig(bool);
  void saveConfig();
  void gotoSnapshot(int index);
  void nextSnapshot();
  void prevSnapshot();

  bool openLog();
  void reopenLog();
  void runCore();
  void runCoreFrame(int frame, bool start = false, bool end = false);
  void rerunCore();
  void runLog();
  void setCore(bool value);
  void runStream();
  void stopStream();

  bool openRecent();

  void openFilesWnd();
  void openTeamWnd();
  void openLogEditorWnd();
  void openLogSelectWnd();
  void openLogWnd();
  void openMotionWnd();
  void openPlotWnd();
  void openVisionWnd();
  void openWorldWnd();
  void openJointsWnd();
  void openWalkWnd();
  void openSensorWnd();
  void openStateWnd();
  void openCameraWnd();

  void handleStreamFrame();

  void remoteRestartInterpreter();
  void updateConfigFile();
  void updateCalibrationFile();
  void readCalibrationFile();

Q_SIGNALS:
  void newStreamFrame();
  void newLogFrame(int);
  void newLogLoaded(LogViewer*);
  void runningCore();
  void setStreaming(bool);
  void loadingConfig(const ToolConfig& config);
  void savingConfig(ToolConfig& config);
  void annotationsUpdated(AnnotationGroup* annotations);
};
#endif
