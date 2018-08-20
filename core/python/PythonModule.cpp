#include <cstdlib>
#include "PythonModule.h"



PythonModule::PythonModule(VisionCore* core) : InterpreterModule(core), pyface_(NULL) {
  timer_.setMessage("Python frame");
}

PythonModule::~PythonModule(){
  if (pyface_ != NULL) 
    delete pyface_;
}

void PythonModule::initSpecificModule() {
  start(); 
}

void PythonModule::updateModuleMemory(MemoryFrame *memory) {
  memory_ = memory;
  specifyMemoryBlocks();

  // if python is broken, don't run
  if (!checkPython()) return;
  pyface_->Execute("initMemory()");
}

bool PythonModule::checkPython() {
  if(!is_ok_) {
    if(pyface_) {
      delete pyface_;
      pyface_ = NULL;
    }
    return false;
  }
  return true;
}

void PythonModule::processFrame() {
  // restart python if requested
  if (restart_requested_) {
    restart();
  }
  
  if(!is_ok_) return;
  call("processFrame()");
}

void PythonModule::call(std::string cmd) {
  if(!checkPython()) return;
  pyface_->Execute(cmd);
}

void PythonModule::start() {
  // Read configuration file and place appropriate values in memory
  readConfig();
  if(pyface_ == NULL) {
    pyface_ = new PythonInterface();
    pyface_->Init(core_);
  }
  restart_requested_ = false;
}

void PythonModule::doStrategyCalculations() {
  //TODO: fill in
}

void PythonModule::restart() {
  pyface_->Finalize();
  pyface_->Init(core_);
  start();
}

void PythonModule::initFromMemory() {
  call("initNonMemory()");
}

void PythonModule::processBehaviorFrame() {
  call("processBehaviorFrame()");
}

void PythonModule::saveKickParameters(const KickParameters& kp) {
  //TODO: fill in
}

void PythonModule::runBehavior(std::string behavior) {
  call("runBehavior('" + behavior + "')");
}
