#ifndef UTMAINWND_H
#define UTMAINWND_H

#include <QtGui/qmainwindow.h>
#include <QFileDialog>

#include <common/ToolPacket.h>
#include <memory/LogWriter.h>
#include <memory/MemoryFrame.h>
#include <memory/LogViewer.h>
#include <communications/StreamingMessage.h>
#include <communications/CommInfo.h>
#include <VisionCore.h>

#include <thread>
#include <memory>
#include <vector>
#include <stdio.h>

#include <tool/ToolConfig.h>
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
class TCPClient;
class Arguments;


void server(UTMainWnd *main);

class UTMainWnd : public QMainWindow, public Ui_UTNaoTool {
Q_OBJECT
public:
  UTMainWnd(const Arguments& args);
  ~UTMainWnd();

  void sendUDPCommand(QString address, ToolPacket packet);
  void sendUDPCommandToCurrent(ToolPacket packet);
  QString getCurrentAddress();

  void setFrameBounds(int start = 0, int end = -1);
  static UTMainWnd* inst() { return instance_; }
  static VisionCore* core() { return instance_->visionCore_; }
  static InterpreterModule* interpreter() { return instance_->visionCore_->interpreter_; }
  static std::string dataDirectory() { return std::string(getenv("NAO_HOME")) + "/data"; }

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
  bool isRunningCore() { return runCoreRadio->isChecked(); }
  void processStreamBuffer(const StreamBuffer& buffer);
  void processStreamExit();
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

  std::string rcPath_;
  std::unique_ptr<std::thread> t_sim_stream_;
  int frame_id_;

private:
  ToolConfig config_;
  bool loading_;
  LogWriter* logger_;
  AnnotationGroup* annotations_;
  std::unique_ptr<TCPClient> tcpclient_;

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
