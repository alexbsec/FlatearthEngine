#ifndef _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP
#define _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP

#include "Definitions.hpp"

namespace flatearth {
namespace memory {

class LinearAllocator {
public:
  FEAPI LinearAllocator(uint64 totalSize, void *memory);
  ~LinearAllocator();

  FEAPI void *Allocate(uint64 size, uint64 alignment = 8);
  FEAPI void FreaAll();

  FINLINE uintptr_t AlignForward(uintptr_t address, uint64 alignment) {
    return (address + (alignment - 1)) & ~(alignment - 1);
  }

private:
  uint64 _totalSize;
  uint64 _allocated;
  void *_memory;
  bool _ownsMemory;
};

} // namespace memory
} // namespace flatearth

#endif // _FLATEARHT_ENGINE_MEMORY_LINEAR_ALLOCATOR_HPP
