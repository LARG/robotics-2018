#ifndef PYTHON_INTERFACE
#define PYTHON_INTERFACE

#include <string>
#include <mutex>

class VisionCore;

class PythonInterface {
  public:
    void Init(VisionCore* core);
    void Execute(std::string);
    void Finalize();
    static VisionCore* CORE_INSTANCE;
    static bool EnableOptimization;
  private:
    void* thread_;

    static bool GLOBAL_INITIALIZED;
    static void GlobalInit();
    static void GlobalFinalize();

    static std::mutex CORE_MUTEX, PY_MUTEX;
};

#endif
