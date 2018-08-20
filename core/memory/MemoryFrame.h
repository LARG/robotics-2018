#ifndef MEMORY_3Z94SCHB
#define MEMORY_3Z94SCHB

#include <vector>
#include <string>
#include <memory/MemoryBlock.h>
#include <memory/SharedMemory.h>
#include <memory/PrivateMemory.h>
#include <memory/Lock.h>
#include <common/InterfaceInfo.h>

struct MemoryHeader {
  std::vector<std::string> block_names;
};

class MemoryFrame {
public:
  MemoryFrame(bool use_shared_memory, MemoryOwner::Owner owner, int team_num, int player_num, bool server = false);
  MemoryFrame(const MemoryFrame&);
  ~MemoryFrame();
  MemoryFrame& operator=(const MemoryFrame&);

  void setBlockLogging(const std::string &name,bool log_block);
  MemoryBlock* getBlockPtrByName(const std::string &name);
  const MemoryBlock* getBlockPtrByName(const std::string &name) const;
  bool addBlockByName(const std::string &name, MemoryOwner::Owner owner = MemoryOwner::UNKNOWN);
  
  template <class T>
  bool getBlockByName(T* &ptr, const std::string &name, MemoryOwner::Owner expect_owner) {
    return getBlockByName(ptr, name, true, expect_owner);
  }

  template <class T>
  bool getBlockByName(T* &ptr, const std::string &name, bool output_no_exist = true, MemoryOwner::Owner expect_owner = MemoryOwner::UNKNOWN) {
    MemoryBlock *block = getBlockPtr(name,expect_owner);
    if (block == nullptr) {
      ptr = nullptr;
      if (output_no_exist && name != "speech") // Speech block always fails
        std::cerr << "MemoryFrame::getBlockByName: Error: couldn't get block for name " << name << std::endl;
      return false;
    }
    ptr = (T*)(block);
    if (ptr == nullptr) {
      std::cerr << "MemoryFrame::getBlockByName: Error: got block with unexpected type " << name << std::endl;
      return false;
    }
    return true;
  }

  template <class T>
  bool getOrAddBlockByName(T *&ptr, const std::string &name, MemoryOwner::Owner owner=MemoryOwner::UNKNOWN) {
    MemoryBlock *block = getBlockPtr(name,owner);
    if (block == nullptr) {
      bool res = addBlockByName(name,owner);
      if (!res) {
        std::cerr << "MemoryFrame::getOrAddBlockByName: ERROR: failed to add block for name: " << name << std::endl;
        return false;
      }
    }
    return getBlockByName(ptr,name,true,owner);
  }

  void getBlockNames(std::vector<std::string> &module_names, bool only_log) const;

private:
  template <class T>
  bool addBlock(const std::string &name,T *block) {
    block->owner = previously_added_owner_;
    bool res;
    if (use_shared_memory_)
      res = shared_memory_->addBlock(name,block);
    else
      res = private_memory_->addBlock(name,block);

    return res;
  };

  MemoryBlock* getBlockPtr(const std::string &name, MemoryOwner::Owner expect_owner);
  const MemoryBlock* getBlockPtr(const std::string &name, MemoryOwner::Owner expect_owner) const;

private:
  bool use_shared_memory_;
public:
  MemoryOwner::Owner owner_;
private:
  SharedMemory *shared_memory_;
  PrivateMemory *private_memory_;

  MemoryOwner::Owner previously_added_owner_;

public:
  Lock *vision_lock_;
  Lock *motion_vision_lock_;

public:
  const std::string suffix_;
  std::string data_path_;
  CoreType core_type_;
};

#endif /* end of include guard: MEMORY_3Z94SCHB */
