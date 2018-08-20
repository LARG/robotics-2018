#include "LEDModule.h"

#include <memory/LEDBlock.h>

LEDModule::LEDModule() {
}

void LEDModule::specifyMemoryDependency() {
  requiresMemoryBlock("led_commands");
}

void LEDModule::specifyMemoryBlocks() {
  getOrAddMemoryBlock(leds_,"led_commands");
  
  lightsOff();
  //rightFoot(1,0,0);
}

void LEDModule::lightsOff() {
  leds_->send_leds_=true;
  for (int i=0; i<NUM_LEDS; i++) {
    leds_->values_[i]=0.0f;
  }
}

void LEDModule::lightsOn() {
  leds_->send_leds_=true;
  for (int i=0; i<NUM_LEDS; i++) {
    leds_->values_[i]=1.0f;
  }
}

void LEDModule::partLeftEar(float val, int start, int number) {
  leds_->send_leds_=true;
  for (int i=EarLeft0+start; i<=EarLeft0+start+number; i++) {
    leds_->values_[i]=val;
  }
}

void LEDModule::partRightEar(float val, int start, int number) {
  leds_->send_leds_=true;
  for (int i=EarRight0+start; i<=EarRight0+start+number; i++) {
    leds_->values_[i]=val;
  }
}

void LEDModule::backRightEar(float val) {
  partRightEar(val,5,4);
}

void LEDModule::frontRightEar(float val) {
  partRightEar(val,0,4);
}

void LEDModule::backLeftEar(float val) {
  partLeftEar(val,5,4);
}

void LEDModule::frontLeftEar(float val) {
  partLeftEar(val,0,4);
}

void LEDModule::allLeftEar(float val) {
  leds_->send_leds_=true;
  for (int i=EarLeft0; i<=EarLeft324; i++) {
    leds_->values_[i]=val;
  }
}

void LEDModule::allRightEar(float val) {
  leds_->send_leds_=true;
  for (int i=EarRight0; i<=EarRight324; i++) {
    leds_->values_[i]=val;
  }
}

void LEDModule::allLeftEye(float r, float g, float b) {
  leds_->send_leds_=true;
  
  for (int i=FaceRedLeft0; i<=FaceRedLeft315; i++) {
    leds_->values_[i]=r;
  }
  for (int i=FaceGreenLeft0; i<=FaceGreenLeft315; i++) {
    leds_->values_[i]=g;
  }
  for (int i=FaceBlueLeft0; i<=FaceBlueLeft315; i++) {
    leds_->values_[i]=b;
  }
  
}

void LEDModule::allTopRightEye(float r, float g, float b) {
  allRangeEye(6, 4, true, r, g, b);
}

void LEDModule::allBottomRightEye(float r, float g, float b) {
  allRangeEye(2, 4, true, r, g, b);
}

void LEDModule::allTopLeftEye(float r, float g, float b) {
  allRangeEye(6, 4, false, r, g, b);
}

void LEDModule::allBottomLeftEye(float r, float g, float b) {
  allRangeEye(2, 4, false, r, g, b);
}

void LEDModule::allRangeEye(int startOffset, int count, bool right, float r, float g, float b) {
  leds_->send_leds_ = true;

  int startR = FaceRedLeft0, startG = FaceGreenLeft0, startB = FaceBlueLeft0;
  if(right) {
    startR = FaceRedRight0, startG = FaceGreenRight0, startB = FaceBlueRight0;
  }

  for(int i = 0; i < count; i++) {
    leds_->values_[startR + (startOffset + i) % LEDS_PER_EYE] = r;
    leds_->values_[startG + (startOffset + i) % LEDS_PER_EYE] = g;
    leds_->values_[startB + (startOffset + i) % LEDS_PER_EYE] = b;
  }
}

void LEDModule::allRightEye(float r, float g, float b) {
  leds_->send_leds_=true;
  
  for (int i=FaceRedRight0; i<=FaceRedRight315; i++) {
    leds_->values_[i]=r;
  }
  for (int i=FaceGreenRight0; i<=FaceGreenRight315; i++) {
    leds_->values_[i]=g;
  }
  for (int i=FaceBlueRight0; i<=FaceBlueRight315; i++) {
    leds_->values_[i]=b;
  }
  
}

void LEDModule::chest(float r, float g, float b) {
  leds_->send_leds_=true;
  leds_->values_[ChestRed] = r;
  leds_->values_[ChestGreen] = g;
  leds_->values_[ChestBlue] = b;
}

void LEDModule::leftFoot(float r, float g, float b) {
  leds_->send_leds_=true;
  leds_->values_[LFootRed] = r;
  leds_->values_[LFootGreen] = g;
  leds_->values_[LFootBlue] = b;
}

void LEDModule::rightFoot(float r, float g, float b) {
  leds_->send_leds_=true;
  leds_->values_[RFootRed] = r;
  leds_->values_[RFootGreen] = g;
  leds_->values_[RFootBlue] = b;
}

void LEDModule::headFront() {
  leds_->send_leds_=true;
  leds_->values_[RHead0] = 1;
  leds_->values_[RHead1] = 1;
  leds_->values_[LHead0] = 1;
  leds_->values_[LHead1] = 1;
}

void LEDModule::headMiddle() {
  leds_->send_leds_=true;
  leds_->values_[RHead2] = 1;
  leds_->values_[LHead2] = 1;
}

void LEDModule::headRear() {
  leds_->send_leds_=true;
  leds_->values_[RHead3] = 1;
  leds_->values_[RHead4] = 1;
  leds_->values_[RHead5] = 1;
  leds_->values_[LHead3] = 1;
  leds_->values_[LHead4] = 1;
  leds_->values_[LHead5] = 1;
}

void LEDModule::headRight() {
  leds_->send_leds_=true;
  leds_->values_[RHead0] = 1;
  leds_->values_[RHead1] = 1;
  leds_->values_[RHead2] = 1;
  leds_->values_[RHead3] = 1;
  leds_->values_[RHead4] = 1;
  leds_->values_[RHead5] = 1;
}

void LEDModule::headLeft() {
  leds_->send_leds_=true;
  leds_->values_[LHead0] = 1;
  leds_->values_[LHead1] = 1;
  leds_->values_[LHead2] = 1;
  leds_->values_[LHead3] = 1;
  leds_->values_[LHead4] = 1;
  leds_->values_[LHead5] = 1;
}

void LEDModule::headAll() {
  leds_->send_leds_=true;
  leds_->values_[RHead0] = 1;
  leds_->values_[RHead1] = 1;
  leds_->values_[RHead2] = 1;
  leds_->values_[RHead3] = 1;
  leds_->values_[RHead4] = 1;
  leds_->values_[RHead5] = 1;
  leds_->values_[LHead0] = 1;
  leds_->values_[LHead1] = 1;
  leds_->values_[LHead2] = 1;
  leds_->values_[LHead3] = 1;
  leds_->values_[LHead4] = 1;
  leds_->values_[LHead5] = 1;
}

void LEDModule::headOff() {
  leds_->send_leds_=true;
  leds_->values_[RHead0] = 0;
  leds_->values_[RHead1] = 0;
  leds_->values_[RHead2] = 0;
  leds_->values_[RHead3] = 0;
  leds_->values_[RHead4] = 0;
  leds_->values_[RHead5] = 0;
  leds_->values_[LHead0] = 0;
  leds_->values_[LHead1] = 0;
  leds_->values_[LHead2] = 0;
  leds_->values_[LHead3] = 0;
  leds_->values_[LHead4] = 0;
  leds_->values_[LHead5] = 0;
}

void LEDModule::headCircle() {
  leds_->send_leds_=true;
  static int i = -1;
  i++;
  headOff();
  const int numHeadLEDs = RHead0 - LHead0 + 1;
  const int framesPerLED = 1;
  int currentIndex = LHead0 + ((i / framesPerLED) % numHeadLEDs);
  leds_->values_[currentIndex] = 1;
}
