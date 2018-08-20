#include <QApplication>
#include <iostream>
#include <common/Profiling.h>

#include "UTMainWnd.h" 
#include "FilesWindow.h"
#include <InterpreterModule.h>
#include "simulation/LocalizationSimulation.h"
#include "simulation/BehaviorSimulation.h"

namespace {

  bool displayMotionWindow = false;
  bool displayVisionWindow = false;
  bool displayFilesWindow = false;
  bool displayWorldWindow = false;
  bool displayPlotWindow = false;
  bool displayTeamWindow = false;
  bool displayLogWindow = false;
  bool openRecent = false;
  bool runCore = false;
  bool locOnly = false;
  bool visOnly = false;
  bool disableUI = false;
  std::string logFile;
  int logindex = -1;
  QString IP;

  int startFrame = 0;
  int endFrame = -1;
  bool locSim = false;
  bool behaviorSim = false;
}

void displayHelp() {
  std::cout << "\n\nOptions :\n";
  std::cout << "\t-o <file>\topens log file\n";
  std::cout << "\t-m\topens motion window\n";
  std::cout << "\t-v\topens vision window\n";
  std::cout << "\t-f\topens files window\n";
  //std::cout << "\t-c\topens control window\n";
  std::cout << "\t-w\topens world window\n";
  std::cout << "\t-g\topens log window\n";
  //std::cout << "\t-v\topens vision window\n";
  //std::cout << "\t-k\topens kinematics window\n";
  std::cout << "\t-z\tdefaults to run core\n";
  std::cout << "\t-r\topens most recent log\n";
  std::cout << "\t-t\topens team cfg window\n";
  std::cout << "\t-s\tselect start frame\n";
  std::cout << "\t-e\tselect end frame\n";
  std::cout << "\t-i <0-9>\tinitializes with IP set to 192.168.25.9x\n";
  std::cout << "\t-c <0-9>\tinitializes with IP set to 11.0.1.9x\n";
  std::cout << "\t-l\tenables localization only\n";
  std::cout << "\t-^\tenables vision only\n";
  std::cout << "\t-d\t Disables the UI for debugging\n";
  //std::cout << "\t-n\tdo NOT run core (for slow machines)\n";
  std::cout << "\t-h -?\t display this help\n\n";
}

int getParameters(int argc, char **argv) {
  char ch;
  const char* optflags = "gmpvfwrto:i:c:s:e:h?:zl^dxb";
  while(-1 != (ch = getopt(argc, argv, optflags))) {
    switch(ch) {
    case 'b':
      behaviorSim = true;
      break;
    case 'x':
      locSim = true;
      break;
    case 'm':
      displayMotionWindow = true;
      break;
    case 'p':
      displayPlotWindow = true;
      break;
    case 'v':
      displayVisionWindow = true;
      break;
    case 'f':
      displayFilesWindow = true;
      break;
    case 'w':
      displayWorldWindow = true;
      break;
    case 't':
      displayTeamWindow = true;
      break;
    case 'g':
      displayLogWindow = true;
      break;
    case 'o':
      logFile = std::string(optarg);
      break;
    case 'r':
      openRecent = true;
      break;
    case 'i':
      IP = "192.168.25.9"+QString(optarg);
      break;
    case 'c':
      IP = "11.0.1.9"+QString(optarg);
      break;
    case 's':
      startFrame = atoi(optarg);
      break;
    case 'e':
      endFrame = atoi(optarg);
      break;
    case 'z':
      runCore = true;
      break;
    case 'l':
      locOnly = true;
      break;
    case '^':
      visOnly = true;
      break;
    case 'd':
      disableUI = true;
      break;
    case 'h':
    case '?':
      displayHelp();
      return 0;
    }
  }
  return 1;
}

void runBehaviorSim() {
  // 10 players. Enable localization to generate randomness in experiments
  BehaviorSimulation* simulation = new BehaviorSimulation(10, false, true);

  while (!simulation->complete()) {
    simulation->simulationStep();
  }
  std::cout << simulation->simBlueScore << " " << simulation->simRedScore << std::endl;

  delete simulation;
}

void runLocSim() {
  std::vector<AgentError> errors;
  for(int i = 0; i < 100; i++) {
    tic();
    auto seed = rand();
    auto path = SimulationPath::generate(10, seed);
    auto psim = new LocalizationSimulation(LocSimAgent::Default);
    fprintf(stderr, "Running simulation with seed %i\n", seed);
    psim->setPath(path);
    auto func = [] (LocalizationSimulation* sim) {
      while(!sim->complete()) {
        sim->simulationStep();
      }
    };
    auto pthread = std::thread(func, psim);
    pthread.join();
    fprintf(stderr, "Sim time: %2.2f seconds\n", toc());
    psim->printError();
    errors.push_back(psim->getError(LocSimAgent::Default));
    auto avg = AgentError::average(errors);
    fprintf(stderr, "Avg dist: %2.2f, Avg rot: %2.2f, Avg steps: %2.2f\n",
      avg.dist, avg.rot, avg.steps
    );
    fprintf(stderr, "----------------------------------------------------------\n");
  }
}

int main(int argc, char **argv) {

  if (!getParameters(argc, argv))
    return 0;

  if (behaviorSim) {
    runBehaviorSim();
    return 0;
  }

  if(locSim) {
    runLocSim();
    return 0;
  }

  QApplication a(argc, argv);

  // UTMainWnd* main_wnd;
  std::unique_ptr<UTMainWnd> main_wnd;
  UTMainWnd::reload_ = openRecent;
  if (logFile.empty()) {
    main_wnd = std::make_unique<UTMainWnd>();
    // main_wnd = new UTMainWnd(NULL);
  } else {
    main_wnd = std::make_unique<UTMainWnd>(logFile.c_str());
    // main_wnd = new UTMainWnd(logFile.c_str());
  }

  // main_wnd->setFrameRange(startFrame, endFrame); // Disabled this since there are UI components now - JM 04/17/15

  if (IP.size() > 0){
    std::cout << "IP : " << IP.toStdString() << std::endl;
    main_wnd->filesWnd_->setCurrentLocation(IP);
  }

  main_wnd->show();
  
  if (displayMotionWindow)
    main_wnd->openMotionWnd();
  if (displayPlotWindow)
    main_wnd->openPlotWnd();
  if (displayVisionWindow)
    main_wnd->openVisionWnd();
  if (displayFilesWindow)
    main_wnd->openFilesWnd(); 
  if (displayWorldWindow)
    main_wnd->openWorldWnd();
  if (displayTeamWindow)
    main_wnd->openTeamWnd();
  if (displayLogWindow)
    main_wnd->openLogWnd();
  if (locOnly)
    main_wnd->localizationOnlyRadioButton->setChecked(true);
  if (visOnly)
    main_wnd->visionOnlyRadioButton->setChecked(true);
  if(runCore)
    main_wnd->setCore(true);
  if(logindex >= 0)
    main_wnd->gotoSnapshot(logindex);

  if(!disableUI) a.exec();

  // delete main_wnd;
  return 0;
}
