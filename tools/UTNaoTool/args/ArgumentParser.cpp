#include <tool/args/ArgumentParser.h>
#include <tool/args/MultiValueToken.h>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <iostream>

Arguments ArgumentParser::Parse(int argc, char **argv) {
  using namespace boost;
  using namespace boost::program_options;
  using namespace std;
  Arguments args;

  options_description logging("Logging Options");
  logging.add_options()
    ("help,h", "Show this help message")
    ("open-log-path,o", value(&args.log_path)->default_value(""), 
     "Load the specified log file during startup.")
    ("open-recent-log,r", bool_switch(&args.open_recent)->default_value(false), 
     "Open the most recent log based on the modification date.")
    ("open-previous-log,p", bool_switch(&args.open_previous)->default_value(false), "Reopen the last log that was opened in the tool.")
    ("bypass-vision", bool_switch(&args.bypass_vision)->default_value(false), 
     "Bypass vision processing when replaying logs or running core. This is useful when no raw "
     "or segmented image is available in the log, or to improve performance when debugging non-vision components.")
    ("vision-only", bool_switch(&args.vision_only)->default_value(false), 
     "View and process only the Vision Module and supporting modules when replaying logs or running core."
     "This is useful when debugging the Vision Module.")
    ("run-core,z", bool_switch(&args.run_core)->default_value(false), "Run core modules on loaded logs.")
    ("disable-gui,d", bool_switch(&args.disable_gui)->default_value(false), "Disable the tool's Graphical User Interface.")
    ("log-bounds,b", FixedTokensValue<int,2>(&args.frame_bounds)->default_value(0,-1), 
     "In some circumstances it is necessary to restrict the number of frames considered at once for a log, "
     "particularly with large logs that can be slow to process. When frame bounds are set, "
     "the log is treated as if the start and end frames were the provided bounds; all other "
     "frames are ignored during viewing and processing.")
  ;

  options_description config("Tool Configuration");
  config.add_options()
    ("ip-address,i", value(&args.ip_address)->default_value(""), "Specify your robot's IP address for logging, streaming, deploying, and gameplay.")
  ;

  options_description secondary("Secondary Processing Tools");
  secondary.add_options()
    ("behavior-sim", bool_switch(&args.behavior_sim)->default_value(false), "Run the behavior simulator.")
    ("loc-sim", bool_switch(&args.localization_sim)->default_value(false), "Run the localization simulator.")
    ("log-server", bool_switch(&args.log_server)->default_value(false), "Simulate streaming by echoing a log over a TCP server.")
    ("loop-server", bool_switch(&args.loop_server)->default_value(false), "Loop through the specified log until the server is forcibly shut down.")
  ;

  options_description windows("Window Launchers");
  windows.add_options()
    ("world-window", bool_switch(&args.world_window)->default_value(false), 
     "Open the World Window during tool startup.")
    ("vision-window", bool_switch(&args.vision_window)->default_value(false), 
     "Open the Vision Window during tool startup.")
    ("files-window", bool_switch(&args.files_window)->default_value(false), 
     "Open the Files Window during tool startup.")
    ("motion-window", bool_switch(&args.motion_window)->default_value(false), 
     "Open the Motion Window during tool startup.")
    ("text-log-window", bool_switch(&args.text_log_window)->default_value(false), 
     "Open the Text LogViewer Window during tool startup.")
    ("memory-select-window", bool_switch(&args.memory_select_window)->default_value(false), 
     "Open the Block Select Window during tool startup.")
    ("joints-window", bool_switch(&args.joints_window)->default_value(false), 
     "Open the Joints Window during tool startup.")
    ("state-window", bool_switch(&args.state_window)->default_value(false), 
     "Open the State Window during tool startup.")
    ("camera-window", bool_switch(&args.camera_window)->default_value(false), 
     "Open the Camera Window during tool startup.")
    ("sensors-window", bool_switch(&args.sensors_window)->default_value(false), 
     "Open the Sensors Window during tool startup.")
    ("plot-window", bool_switch(&args.plot_window)->default_value(false), 
     "Open the Plot Window during tool startup.")
    ("team-config-window", bool_switch(&args.team_config_window)->default_value(false), 
     "Open the Team Config Window during tool startup.")
    ("walk-window", bool_switch(&args.walk_window)->default_value(false), 
     "Open the Walk Window during tool startup.")
    ("log-edit-window", bool_switch(&args.log_edit_window)->default_value(false), 
     "Open the LogViewer Edit Window during tool startup.")
    ("audio-window", bool_switch(&args.audio_window)->default_value(false), 
     "Open the Audio Window during tool startup.")
  ;
  
  options_description combined;
  combined.add(config).add(logging).add(secondary).add(windows);

  variables_map vm;
  store(command_line_parser(argc, argv).options(combined).run(), vm);
  notify(vm);

  if(vm.count("help")) {
    cout << "Usage: tool [options]\n";
    cout << "\n";
    cout << combined << "\n";
    exit(0);
  }
  return args;
}
