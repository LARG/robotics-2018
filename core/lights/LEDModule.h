#ifndef LED_MODULE_H
#define LED_MODULE_H

#include <Module.h>

class LEDBlock;

class LEDModule: public Module {
 public:
  LEDModule();

  void specifyMemoryDependency();
  void specifyMemoryBlocks();

  void lightsOff();
  void lightsOn();

  void allLeftEar(float val);
  void allRightEar(float val);

  void partLeftEar(float val, int start, int number);
  void partRightEar(float val, int start, int number);
  
  void backLeftEar(float val);
  void frontLeftEar(float val);
  void backRightEar(float val);
  void frontRightEar(float val);

  void allTopRightEye(float r, float g, float b);
  void allBottomRightEye(float r, float g, float b);
  void allTopLeftEye(float r, float g, float b);
  void allBottomLeftEye(float r, float g, float b);
  void allRangeEye(int startOffset, int count, bool right, float r, float g, float b);
  void allLeftEye(float r, float g, float b);
  void allRightEye(float r, float g, float b);

  void chest(float r, float g, float b);
  void leftFoot(float r, float g, float b);
  void rightFoot(float r, float g, float b);

  void headFront();
  void headMiddle();
  void headRear();
  void headLeft();
  void headRight();
  void headAll();
  void headOff();
  void headCircle();

 private:
  LEDBlock *leds_;
};

#endif /* end of include guard: LED_MODULE */
