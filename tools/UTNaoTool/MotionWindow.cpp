#include <tool/MotionWindow.h>

using namespace std;

MotionWindow::MotionWindow(QMainWindow* pa) : ConfigWindow(pa) {
  setupUi(this);
  connect(keyframes_, SIGNAL(playingSequence(const Keyframe&, const Keyframe&, int)), motion_, SLOT(drawSequence(const Keyframe&, const Keyframe&, int)));
  connect(keyframes_, SIGNAL(showingKeyframe(const Keyframe&)), motion_, SLOT(drawKeyframe(const Keyframe&)));
  connect(keyframes_, SIGNAL(updatedSupportBase(SupportBase)), motion_, SLOT(setSupportBase(SupportBase)));
}

void MotionWindow::updateMemory(MemoryFrame* mem) {
  motion_->updateMemory(mem);
  motion_->update();
  cache_ = MemoryCache::read(mem);
  keyframes_->updateMemory(cache_);
}
