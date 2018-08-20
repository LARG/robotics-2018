#ifndef PRIVATEMEMORY_DMIY3W1X
#define PRIVATEMEMORY_DMIY3W1X

#include <vector>
#include <map>
#include <string>
#include <memory/MemoryBlock.h>
#include <memory/AbstractMemory.h>

typedef std::map<const std::string,MemoryBlock*> MemMap;

class PrivateMemory : public AbstractMemory {
public:
  PrivateMemory();
  PrivateMemory(const PrivateMemory &mem);
  virtual ~PrivateMemory();
  bool addBlock(const std::string &name,MemoryBlock *block);
  MemoryBlock* getBlockPtr(const std::string &name);
  const MemoryBlock* getBlockPtr(const std::string &name) const;
  void getBlockNames(std::vector<std::string> &module_names,bool only_log, MemoryOwner::Owner for_owner) const;

protected:
  MemMap blocks_;
private:
  bool is_copy_;
};

#endif /* end of include guard: PRIVATEMEMORY_DMIY3W1X */
