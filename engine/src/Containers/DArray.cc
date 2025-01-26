#include "DArray.hpp"

#include <cstring>

namespace flatearth {
namespace containers {

template <typename T>
DArray<T>::DArray(uint64 stride)
    : _capacity(DARRAY_DEFAULT_SIZE), _length(0), _stride(stride) {
  InitializeMemory();
}

template <typename T>
DArray<T>::DArray(uint64 capacity, uint64 stride)
    : _capacity(capacity), _length(0), _stride(stride) {
  InitializeMemory();
}

template <typename T> uint64 DArray<T>::GetCapacity() const {
  return _capacity;
}

template <typename T> void DArray<T>::SetCapacity(uint64 capacity) {
  _capacity = capacity;
}

template <typename T> uint64 DArray<T>::GetLength() const { return _length; }

template <typename T> void DArray<T>::SetLength(uint64 length) {
  _length = length;
}

template <typename T> uint64 DArray<T>::GetStride() const { return _stride; }

template <typename T> void DArray<T>::SetStride(uint64 stride) {
  _stride = stride;
}

// PRIVATE

template <typename T> void DArray<T>::InitializeMemory() {
  uint64 headerSize = DARRAY_FIELD_LENGTH * sizeof(uint64);
  uint64 arraySize = _capacity * _stride;
  _array = std::unique_ptr<T[], decltype(&core::memory::MemoryManager::Free)>(
      reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
          headerSize + arraySize, core::memory::MEMORY_TAG_DARRAY)),
      core::memory::MemoryManager::Free);

  core::memory::MemoryManager::SetMemory(_array.get(), 0,
                                         headerSize + arraySize);
}


} // namespace containers
} // namespace flatearth
