#include <QApplication>
#include <args/ArgumentParser.h>
#include <tool/UTMainWnd.h>
#include <tool/CommandLineProcessor.h>

using CLP = CommandLineProcessor;

int main(int argc, char **argv) {
  auto args = ArgumentParser::Parse(argc, argv);

  if (args.behavior_sim) 
    return CLP::runBehaviorSim();
  if(args.localization_sim)
    return CLP::runLocalizationSim();
  if(args.log_server)
    return CLP::runLogServer(args.log_path, args.loop_server);

  QApplication a(argc, argv);

  auto main = std::make_unique<UTMainWnd>(args);

  main->show();
  
  if(args.world_window)
    main->openWorldWnd();
  if(args.vision_window)
    main->openVisionWnd();
  if(args.files_window)
    main->openFilesWnd();
  if(args.text_log_window)
    main->openLogWnd();
  if(args.memory_select_window)
    main->openLogSelectWnd();
  if(args.joints_window)
    main->openJointsWnd();
  if(args.state_window)
    main->openStateWnd();
  if(args.camera_window)
    main->openCameraWnd();
  if(args.sensors_window)
    main->openSensorWnd();
  if(args.plot_window)
    main->openPlotWnd();
  if(args.team_config_window)
    main->openTeamWnd();
  if(args.walk_window)
    main->openWalkWnd();
  if(args.log_edit_window)
    main->openLogEditorWnd();

  if(!args.disable_gui) a.exec();

  return 0;
}
