#pragma once

#include <Module.h>

class VisionCore;

class AudioModule: public Module {
  public:

    AudioModule();

    void specifyMemoryDependency();
    void specifyMemoryBlocks();
    void initSpecificModule();
    void initFromMemory();
    void processFrame();
};
