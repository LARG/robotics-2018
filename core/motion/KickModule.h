#pragma once

#include <Module.h>
#include <common/RobotInfo.h>
#include <memory/MemoryCache.h>

class Keyframe;
class KeyframeSequence;

class KickModule : public Module {
  public:
    KickModule();
    void initSpecificModule();
    void specifyMemoryDependency();
    void specifyMemoryBlocks();

    void processFrame();

  protected:
    ENUM(KickState,
      Initial,
      Running,
      Finished
    );
    void start();
    void finish();
    bool finished();
    void moveBetweenKeyframes(const Keyframe& start, const Keyframe& finish, int cframe);
    void initStiffness();
    void performKick();
    void moveToInitial(const Keyframe& keyframe, int cframe);
    bool reachedKeyframe(const Keyframe& keyframe);
  private:
    KickState state_;
    MemoryCache cache_;
    KeyframeSequence* sequence_;
    Keyframe* initial_;
    int frames_;
    int keyframe_;
};
