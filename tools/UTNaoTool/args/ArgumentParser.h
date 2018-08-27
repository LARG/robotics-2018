#pragma once

#include <string>
#include <vector>

struct Arguments {
  std::string log_path;
  bool
    run_core,
    open_recent,
    open_previous,
    bypass_vision,
    vision_only,
    disable_gui
  ;
  std::vector<int> frame_bounds;
  std::string ip_address;
  bool behavior_sim, localization_sim, log_server, loop_server;
  bool
    world_window,
    vision_window,
    files_window,
    motion_window,
    text_log_window,
    memory_select_window,
    joints_window,
    state_window,
    camera_window,
    sensors_window,
    plot_window,
    team_config_window,
    walk_window,
    log_edit_window,
    audio_window
  ;
};

class ArgumentParser {
  public:
    static Arguments Parse(int argc, char **argv);
};
