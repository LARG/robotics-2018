#ifndef WORLD_WINDOW_H
#define WORLD_WINDOW_H

#include <QtGui>
#include <QWidget>

#include "ui_WorldWindow.h"
#include <tool/ConfigWindow.h>
#include <memory/MemoryFrame.h>
#include <iostream>
#include <tool/SimulatorConfig.h>

class AnnotationGroup;

class WorldWindow : public ConfigWindow, public Ui_WorldWindow {
 Q_OBJECT

  public:
    WorldWindow(QMainWindow* pa);
    void updateMemory(MemoryFrame* mem);
    void updateMemory(MemoryCache cache);
    ENUM(WorldMode,
      NoDraw,
      Simple,
      Localization,
      Vision,
      Live,
      BehaviorSim,
      BehaviorSimLoc,
      IsolatedBehaviorSim,
      IsolatedBehaviorSimLoc,
      LocalizationSim,
      GoalieSim
    );
    Qt::KeyboardModifiers modifiers() { return world->modifiers(); }
  protected:
    bool inSimMode();
    bool inLiveMode();
    void startLiveMode();
    void stopLiveMode();
    void showEvent(QShowEvent* event) override;
  public slots:
    void loadConfig(const ToolConfig& config);
    void saveConfig(ToolConfig& config);
    void updateAnnotations(AnnotationGroup* annotations);
    void pause() { play_ = false; updateDisplay(); }
  private slots:
    void fieldClicked(Point2D pos, Qt::MouseButton button);
    void fieldHovered(Point2D pos);
    void updateLiveMode();
    void setMode(WorldMode mode, bool restart = false);
    void updateDisplay(bool = true);
    void updateSimulationState();
    void updateSimulationView();

    void skip();
    void play() { play_ = true; updateDisplay(); }
    void forward();
    void back();
    void restart(LogViewer*) { restart(); }
    void restart();
    void handleStreaming(bool streaming);

    void controlsChanged();
    void displayOptionsChanged(bool);

    void startSimulation();
    void stopSimulation();
  signals:
    void modeChanged(WorldMode mode);
    void nextSnapshot();
    void prevSnapshot();
    void frameLoaded(int frame);
    void annotationsUpdated(AnnotationGroup* annotations);
  private:
    void setDisplay(bool value);
    std::map<GLDrawer::DisplayOption, bool> display_;
    std::map<GLDrawer::DisplayOption, QCheckBox*> displayBoxes_;
    std::vector<WorldMode> modes_;
    int player_;
    
    std::unique_ptr<Simulation> simulation_;
    QTimer *state_timer_, *view_timer_;
    bool play_;
    WorldConfig wconfig_;
    bool updating_;
    std::unique_ptr<VisionCore> livecore_;
    MemoryCache livecache_;
    QTimer* livetimer_;
    AnnotationGroup* annotations_;
    SimulatorConfig simconfig_;
    bool forward_ = false;
};

#endif
