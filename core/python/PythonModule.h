#ifndef PYTHON_99KDYFIX5
#define PYTHON_99KDYFIX5

#include <InterpreterModule.h>
#include <python/PythonInterface.h>

class PythonModule: public InterpreterModule {
public:
  PythonModule(VisionCore* core);
  ~PythonModule();

  void initSpecificModule();
  void initFromMemory();
  void updateModuleMemory(MemoryFrame *memory);
  void processFrame();
  void processBehaviorFrame();
  void start();
  void restart();
  void doStrategyCalculations();
  void saveKickParameters(const KickParameters& kp);
  void runBehavior(std::string behavior);

private:
  bool checkPython();
  void call(std::string cmd);
  PythonInterface* pyface_;
};

#endif /* end of include guard: VISION_99KDYIX5 */
