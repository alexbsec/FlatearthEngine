#include "LinearAllocator.hpp"

#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include <cstdint>

namespace flatearth {
namespace memory {

LinearAllocator::LinearAllocator(uint64 totalSize, void *memory)
    : _totalSize(totalSize), _allocated(0), _memory(memory),
      _ownsMemory(memory == nullptr) {}

LinearAllocator::~LinearAllocator() {
  _allocated = 0;
  if (_memory) {
    core::memory::MemoryManager::Free(
        _memory, _totalSize, core::memory::MEMORY_TAG_LINEAR_ALLOCATOR);
  }

  _totalSize = 0;
  _ownsMemory = FeFalse;
  _memory = nullptr;
}

void *LinearAllocator::Allocate(uint64 size, uint64 alignment) {
  if (!_memory) {
    // This should never happen!
    FERROR("LinearAllocator::Allocate(): Provided allocator not initialized!");
    return nullptr;
  }

  uintptr_t currentAddr = reinterpret_cast<uintptr_t>(_memory) + _allocated;
  uintptr_t alignedAddr = AlignForward(currentAddr, alignment);
  uint64 adjustment = alignedAddr - currentAddr;

  if (_allocated + size + adjustment > _totalSize) {
    uint64 remaining = _totalSize - _allocated;
    FERROR("LinearAllocator::Allocate(): Tried to allocate %lluB (with %lluB "
           "alignment), only %lluB remaining",
           size, adjustment, remaining);
    return nullptr;
  }

  _allocated += adjustment + size;
  return reinterpret_cast<void *>(alignedAddr);
}

void LinearAllocator::FreaAll() {
  if (!_memory) {
    return;
  }

  _allocated = 0;
  core::memory::MemoryManager::ZeroMemory(_memory, _totalSize);
}

} // namespace memory
} // namespace flatearth
