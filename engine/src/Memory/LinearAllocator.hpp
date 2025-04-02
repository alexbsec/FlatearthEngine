#ifndef _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP
#define _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP

#include "Definitions.hpp"
#include "Core/FeMemory.hpp"

namespace flatearth {
namespace memory {

class LinearAllocator {
public:
  FEAPI LinearAllocator(uint64 totalSize, void *memory, flatearth::core::memory::MemoryTag tag);
  ~LinearAllocator();

  FEAPI void *Allocate(uint64 size);
  FEAPI void FreaAll();

private:
  uint64 _totalSize;
  uint64 _allocated;
  void *_memory;
  bool _ownsMemory;
};

} // namespace memory
} // namespace flatearth

#endif // _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP
