#include <memory/PrivateMemory.h>
#include <iostream>
#include <memory/MemoryBlockOperations.h>

PrivateMemory::PrivateMemory() : is_copy_(false) {
  //std::cout << "PRIVATE MEMORY CONSTRUCTOR" << std::endl << std::flush;
}

PrivateMemory::PrivateMemory(const PrivateMemory &mem) : is_copy_(true) {
  for (MemMap::const_iterator it = mem.blocks_.begin(); it != mem.blocks_.end(); it++) {
    MemoryBlock *temp = COPY_MEMORY_BLOCK(it->first, it->second);
    if(temp) blocks_.insert(std::pair<std::string,MemoryBlock*>(it->first,temp));
  }
}

PrivateMemory::~PrivateMemory() {
  for (MemMap::iterator it = blocks_.begin(); it != blocks_.end(); it ++) {
    DELETE_MEMORY_BLOCK(it->first, it->second);
  }
  blocks_.clear();
}

bool PrivateMemory::addBlock(const std::string &name,MemoryBlock *block) {
  std::pair<MemMap::iterator,bool> res = blocks_.insert(std::pair<std::string,MemoryBlock*>(name,block));
  // check if the block was inserted
  if (!res.second) {
    std::cerr << "PrivateMemory::addBlock - ERROR ADDING BLOCK " << name << " already exists, Deleting given block" << std::endl;
    delete block;
  }
  return res.second;
}

MemoryBlock* PrivateMemory::getBlockPtr(const std::string &name) {
  MemMap::iterator it = blocks_.find(name);
  if (it == blocks_.end()) // the block isn't in our map
    return NULL;
  else
    return (*it).second;
}

const MemoryBlock* PrivateMemory::getBlockPtr(const std::string &name) const {
  MemMap::const_iterator it = blocks_.find(name);
  if (it == blocks_.end()) // the block isn't in our map
    return NULL;
  else
    return (*it).second;
}

void PrivateMemory::getBlockNames(std::vector<std::string> &module_names, bool only_log, MemoryOwner::Owner for_owner) const {
  for(auto kvp : blocks_) {
    std::string name = kvp.first;
    auto block = kvp.second;
    if((!only_log || block->log_block) && block->checkOwner(name, for_owner, true))
      module_names.push_back(name);
  }
}
