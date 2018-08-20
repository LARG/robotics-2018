#ifndef MODULE_NNIRD0YI
#define MODULE_NNIRD0YI

#include <string>
#include <vector>
#include <iostream>
#include <memory/MemoryFrame.h>
#include <memory/TextLogger.h>

class Module {
public:
  Module ();
  virtual ~Module() = default;
  void init(MemoryFrame *memory, TextLogger* tl);
  virtual void initSpecificModule() {};
  void updateModuleMemory(MemoryFrame *memory);

  // Did all required blocks load 
  bool isMemorySatisfied() { return memory_satisfied_; };

protected:
  void requiresMemoryBlock(const std::string &name);
  void providesMemoryBlock(const std::string &name);
  virtual void specifyMemoryDependency() = 0;
  virtual void specifyMemoryBlocks() = 0;
 
  MemoryFrame *memory_; // Should go private once we remove bhuman walk module
  TextLogger* textlogger;

protected:
  template <class T>
  void getMemoryBlock(T *&ptr, const std::string &name){
    for (unsigned int i = 0; i < required_blocks_.size(); i++) {
      if (required_blocks_[i] == name) {
        bool ok = memory_->getBlockByName(ptr,name);
        if (!ok) memory_satisfied_ = false;
        return;
      }
    }
    std::cerr << "Module::getMemoryBlock - ERROR: tried to get a memory block that was not required: " << name << std::endl;
    return;
  }

  template <class T>
  void getOrAddMemoryBlock(T *&ptr, const std::string &name) {
    for (unsigned int i = 0; i < required_blocks_.size(); i++) {
      if (required_blocks_[i] == name) {
        bool ok = memory_->getOrAddBlockByName(ptr,name);
        if (!ok) memory_satisfied_ = false;
        return;
      }
    }
    std::cerr << "Module::getMemoryBlock - ERROR: tried to get a memory block that was not required: " << name << std::endl;
    return;
  }

private:
  std::vector<std::string> required_blocks_;
  std::vector<std::string> provided_blocks_;

  bool memory_satisfied_;  // Have all required blocks loaded

};

#endif /* end of include guard: MODULE_NNIRD0YI */
