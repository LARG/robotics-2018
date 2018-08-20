#include "SharedMemory.h"


#define SHARED_MEMORY_BASE_NAME "VillaSharedMemory"

const int SharedMemory::MEMORY_SIZE = 10000000;
//const char SharedMemory::SHARED_MEMORY_NAME[] = "VillaSharedMemory";
const char SharedMemory::MEM_MAP_NAME[] = "VillaMemMap";

SharedMemory::SharedMemory(const std::string &suffix, bool server):
  server_(server),
  memory_name_(std::string(SHARED_MEMORY_BASE_NAME) + suffix)
{
  if (server_) {
    // remove the shared memory
    boost::interprocess::shared_memory_object::remove(memory_name_.c_str());
    mem_segment_ = new boost::interprocess::managed_shared_memory(boost::interprocess::create_only,memory_name_.c_str(),MEMORY_SIZE);
    VoidAllocator alloc_inst(mem_segment_->get_segment_manager());
    blocks_ = mem_segment_->construct<SharedMemMap>(MEM_MAP_NAME)(std::less<SharedString>(),alloc_inst);
  }
  else {
    mem_segment_ = new boost::interprocess::managed_shared_memory(boost::interprocess::open_only,memory_name_.c_str());
    blocks_ = mem_segment_->find<SharedMemMap>(MEM_MAP_NAME).first;
  }
}

SharedMemory::~SharedMemory() {
  if (server_) {
    mem_segment_->destroy<SharedMemMap>(MEM_MAP_NAME);
    boost::interprocess::shared_memory_object::remove(memory_name_.c_str());
  }
  delete mem_segment_;
}

MemoryBlock* SharedMemory::getBlockPtr(const std::string &name) {
  SharedString temp(name.c_str(),CharAllocator(mem_segment_->get_segment_manager()));
  SharedMemMap::const_iterator it = blocks_->find(temp);
  if (it == blocks_->end()) // the block isn't in our map
    return NULL;
  else
    return (*it).second.get();
}

const MemoryBlock* SharedMemory::getBlockPtr(const std::string &name) const {
  SharedString temp(name.c_str(),CharAllocator(mem_segment_->get_segment_manager()));
  SharedMemMap::const_iterator it = blocks_->find(temp);
  if (it == blocks_->end()) // the block isn't in our map
    return NULL;
  else
    return (*it).second.get();
}

void SharedMemory::getBlockNames(std::vector<std::string> &module_names, bool only_log, MemoryOwner::Owner for_owner) const {
  for (SharedMemMap::const_iterator it = blocks_->begin(); it != blocks_->end(); it++) {
    if (((!only_log) || (*it).second->log_block) && ((*it).second->checkOwner(std::string((*it).first.c_str()),for_owner,true)))
      module_names.push_back(std::string((*it).first.c_str()));
  }
}


//void SharedMemory::lock() {
//  mem_mutex_->lock();
//}

//void SharedMemory::unlock() {
//  mem_mutex_->unlock();
//}
