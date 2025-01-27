#ifndef _FLATEARTH_ENGINE_FE_MEMORY_HPP
#define _FLATEARTH_ENGINE_FE_MEMORY_HPP

#include "Definitions.hpp"

#include <array>

namespace flatearth {
namespace core {
namespace memory {


enum MemoryTag {
  MEMORY_TAG_UNKNOWN,
  MEMORY_TAG_ARRAY,
  MEMORY_TAG_DARRAY,
  MEMORY_TAG_DICT,
  MEMORY_TAG_RING_QUEUE,
  MEMORY_TAG_BST,
  MEMORY_TAG_STRING,
  MEMORY_TAG_APPLICATION,
  MEMORY_TAG_JOB,
  MEMORY_TAG_TEXTURE,
  MEMORY_TAG_MATERIAL_INSTANCE,
  MEMORY_TAG_RENDERER,
  MEMORY_TAG_GAME,
  MEMORY_TAG_TRANSFORM,
  MEMORY_TAG_ENTITY,
  MEMORY_TAG_ENTITY_NODE,
  MEMORY_TAG_SCENE,

  MEMORY_TAG_MAX_TAGS,
};

struct MemoryBlock {
  uint64 totalAllocated;
  std::array<uint64, MEMORY_TAG_MAX_TAGS> taggedAllocations;
};

class MemoryManager {
public:
  FEAPI MemoryManager();

  FEAPI static void *Allocate(uint64 size, MemoryTag tag);
  FEAPI static void Free(void *block, uint64 size, MemoryTag tag);
  FEAPI static void *ZeroMemory(void *block, uint64 size);
  FEAPI static void *CopyMemory(void *dest, const void *source, uint64 size);
  FEAPI static void *SetMemory(void *dest, sint32 value, uint64 size);
  FEAPI string PrintMemoryUsage() const;

private:
  static void CheckTag(MemoryTag tag, const string& from);
  
  static MemoryBlock _memoryBlock;
};

}
}
}

#endif // _FLATEARTH_ENGINE_FE_MEMORY_HPP
