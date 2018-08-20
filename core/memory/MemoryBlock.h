#ifndef MEMORYBLOCK_4MKDDAED
#define MEMORYBLOCK_4MKDDAED

#include <cstring>
#include <string>
#include <stdlib.h>
#include <memory/StreamBuffer.h>

#include <common/Serialization.h>

struct MemoryBlockHeader {
  unsigned int version;
  unsigned int size;
};

namespace MemoryOwner {
  enum Owner {
    UNKNOWN, // means it didn't get set = BAD
    MOTION,
    VISION,
    INTERFACE,
    IMAGE_CAPTURE,
    SYNC,
    TOOL_MEM, // no blocks are currently owned by the tool, but for the memory itself?
    SHARED
  };
};

class MemoryBlock : public Serializable {
public:
  MemoryBlock();
  virtual ~MemoryBlock() {}
  
  MemoryBlockHeader header;
  bool log_block;
  MemoryOwner::Owner owner;

  mutable const char* directory_;
  mutable bool buffer_logging_;
  mutable int frame_id_;

  virtual MemoryBlock& operator=(const MemoryBlock &that);

#ifndef SWIG
  template <class T>
  constexpr bool is_serializable() const {
    const T* block = static_cast<const T*>(this);
    return block->is_serializable();
  }

  template <class T>
  auto serialize(serialization::serializer& serializer) const {
    const T* block = static_cast<const T*>(this);
    return block->serialize_void(serializer);
  }
  
  template <class T>
  auto deserialize(serialization::data_pointer data_pointer) {
    T* block = static_cast<T*>(this);
    return block->deserialize(data_pointer);
  }
#endif

  bool validateHeader(const StreamBuffer& buffer);
  bool validateHeader(const MemoryBlockHeader& theader);

  bool checkOwner(const std::string &name, MemoryOwner::Owner expect_owner, bool no_exit = false) const;
};

#endif /* end of include guard: MEMORYBLOCK_4MKDDAED */
