#include "Module.h"

Module::Module():
  memory_(NULL),
  memory_satisfied_(true)  // Assume it will work
{
}

void Module::init(MemoryFrame *memory, TextLogger* tl) {
  memory_ = memory;
  textlogger = tl;
  specifyMemoryDependency();
  specifyMemoryBlocks();
  initSpecificModule();
}

void Module::updateModuleMemory(MemoryFrame *memory) {
  memory_ = memory;
  specifyMemoryBlocks();
}

void Module::requiresMemoryBlock(const std::string &name) {
  //I commented this check out as it doesn't work well with the new getOrAdd commands (MQ 3/15/2011)

  //MemoryBlock *block;
  //memory_->getBlockByName(block,name);
  //if (block == NULL)
  //  std::cerr << "Module::requiresMemoyBlock: ERROR: requiring memory block that does not exist: " << name << std::endl;
  required_blocks_.push_back(name);
}

void Module::providesMemoryBlock(const std::string &name) {
  required_blocks_.push_back(name);
  provided_blocks_.push_back(name);
}
