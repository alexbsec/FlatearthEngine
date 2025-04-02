#include "LinearAllocator.hpp"

namespace flatearth {
namespace memory {

#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"

LinearAllocator::LinearAllocator(uint64 totalSize, void *memory,
                                 core::memory::MemoryTag tag)
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

void *LinearAllocator::Allocate(uint64 size) {
  if (!_memory) {
    // This should never happen!
    FERROR("LinearAllocator::Allocate(): Provided allocator not initialized!");
    return nullptr;
  }


  if (_allocated + size > _totalSize) {
    uint64 remaining = _totalSize - _allocated;
    FERROR("LinearAllocator::Allocate(): Tried to allocate %lluB, only %lluB "
           "remaining",
           size, remaining);
    return nullptr;
  }

  void *block = static_cast<uchar *>(_memory) + _allocated;
  _allocated += size;
  return block;
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
