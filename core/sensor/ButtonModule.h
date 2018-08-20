#ifndef BUTTON_MODULE_H
#define BUTTON_MODULE_H

#include <Module.h>

class FrameInfoBlock;
class GameStateBlock;
class RobotStateBlock;
class SensorBlock;
class SpeechBlock;
class CameraBlock;


class ButtonModule: public Module {
 public:
  ButtonModule();

  void specifyMemoryDependency();
  void specifyMemoryBlocks();

  void processButtons();

 private:
  FrameInfoBlock *frame_info_;
  GameStateBlock *game_state_;
  RobotStateBlock *robot_state_;
  SensorBlock *sensors_;
  SpeechBlock *speech_;
  CameraBlock *camera_;

  static const float MAX_CLICK_INTERVAL;
  static const float MIN_CLICK_TIME;
  static const float MAX_CLICK_TIME;

  struct ButtonInfo {
    ButtonInfo(bool allow_multiple):
      start(-1),
      last(-1),
      allow_multiple(allow_multiple)
    {
      reset();
    }

    void reset() {
      presses = 0;
      new_result = false;
    }

    int presses;
    float start;
    float last;
    bool new_result;
    bool allow_multiple;
  };

  ButtonInfo center_;
  ButtonInfo left_bumper_;
  ButtonInfo right_bumper_;
  ButtonInfo head_middle_;

 private:
  void processCenterPresses();
  void processButton(float bump1, float bump2, ButtonInfo &button);
  void sayIP();
};

#endif /* end of include guard: BUTTON_MODULE */
