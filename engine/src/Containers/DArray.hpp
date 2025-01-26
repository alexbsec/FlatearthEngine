#ifndef _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
#define _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP

#include "Definitions.hpp"

#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>

namespace flatearth {
namespace containers {

template <typename T> class DArray {
public:
  // Custom deleter to be able to track memory usage
  static void CustomDeleter(T *ptr) {
    core::memory::MemoryManager::Free(ptr, DARRAY_DEFAULT_SIZE * sizeof(T),
                                      core::memory::MEMORY_TAG_DARRAY);
  }

  // Constants
  FEAPI static constexpr uint64 DARRAY_DEFAULT_SIZE = 1;
  FEAPI static constexpr uchar DARRAY_RESIZE_FACTOR = 2;
  FEAPI static constexpr uint64 DARRAY_FIELD_LENGTH = 3;

  FEAPI DArray(uint64 stride = sizeof(T));
  FEAPI DArray(uint64 capacity, uint64 stride = sizeof(T));
  FEAPI ~DArray() = default;

  FEAPI uint64 GetCapacity() const;
  FEAPI void SetCapacity(uint64 capacity);

  FEAPI uint64 GetLength() const;
  FEAPI void SetLength(uint64 length);

  FEAPI uint64 GetStride() const;
  FEAPI void SetStride(uint64 stride);

  FEAPI void Resize();

  FEAPI void Push(const T &element);
  FEAPI void Pop();

  FEAPI void InsertAt(const T &element, uint64 index);
  FEAPI void PopAt(uint64 index);
  FEAPI void Clear();

  // Operators
  T &operator[](uint64 index);
  const T &operator[](uint64 index) const;

private:
  void InitializeMemory();

  uint64 _capacity;
  uint64 _length;
  uint64 _stride;
  std::unique_ptr<T[], decltype(&CustomDeleter)> _array;
};

template <typename T>
DArray<T>::DArray(uint64 stride)
    : _capacity(DARRAY_DEFAULT_SIZE), _length(0), _stride(stride),
      _array(nullptr, &CustomDeleter) {
  InitializeMemory();
}

template <typename T>
DArray<T>::DArray(uint64 capacity, uint64 stride)
    : _capacity(capacity), _length(0), _stride(stride),
      _array(nullptr, &CustomDeleter) {
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

template <typename T> void DArray<T>::Resize() {
  // Creates a temporary array with the new capacity
  std::unique_ptr<T[], decltype(&CustomDeleter)> temp(
      reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
          DARRAY_RESIZE_FACTOR * _capacity * _stride,
          core::memory::MEMORY_TAG_DARRAY)),
      &CustomDeleter);

  // Copy existing data into the new array
  core::memory::MemoryManager::CopyMemory(temp.get(), _array.get(),
                                          _length * _stride);

  // Update internal state
  _capacity *= DARRAY_RESIZE_FACTOR;
  _array = std::move(temp);
}

template <typename T> void DArray<T>::Push(const T &element) {
  if (_length >= _capacity) {
    Resize();
  }

  // Calculate the destination address using pointer arithmetics
  T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                  (_length * _stride));

  // Copy the element into the calculated memory location and update
  // length
  core::memory::MemoryManager::CopyMemory(dest, &element, _stride);
  _length++;
}

template <typename T> void DArray<T>::Pop() {
  if (_length == 0)
    return;

  // Calculate the destination address of the last element
  T *last = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                  ((_length - 1) * _stride));

  // Explicitly call delete on last element
  last->~T();
  _length--;
}

template <typename T> void DArray<T>::InsertAt(const T &element, uint64 index) {
  if (index > _length) {
    FERROR("DArray<T>::InsertAt(): attempt to insert at invalid index");
    return;
  }

  // Resize if necessary
  if (_length >= _capacity) {
    Resize();
  }

  // Shift elements to the right to make room for the new element
  for (uint64 i = _length; i > index; i--) {
    T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                    (i * _stride));
    T *source = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                      ((i - 1) * _stride));

    // Move each element at the specified index
    core::memory::MemoryManager::CopyMemory(dest, source, _stride);
  }

  // Get the insert location
  T *insertLoc = reinterpret_cast<T *>(
      reinterpret_cast<char *>(_array.get()) + (index * _stride));

  // Insert the new element at the insert location
  core::memory::MemoryManager::CopyMemory(insertLoc, &element, _stride);
  _length++;
}

template <typename T> void DArray<T>::PopAt(uint64 index) {
  if (index >= _length) {
    FERROR("DArray<T>::InsertAt(): attempt to pop at invalid index");
    return;
  }

  // Get a pointer to the element being removed
  T *toRemove = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                      (index * _stride));

  // Call the destructor explicitly to clean up the resource
  toRemove->~T();

  // Shift elements to the left to fill the gap
  for (uint64 i = index; i < _length - 1; i++) {
    T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                    (i * _stride));
    T *source = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                      ((i + 1) * _stride));

    core::memory::MemoryManager::CopyMemory(dest, source, _stride);
  }

  _length--;
}

template <typename T> void DArray<T>::Clear() {
  for (uint64 i = 0; i < _length; i++) {
    T *element = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                       (i * _stride));
    element->~T();
  }

  _length = 0;
}

template <typename T> T &DArray<T>::operator[](uint64 index) {
  if (index >= _length) {
    FERROR("DArray<T>::operator[]: index out of bounds");
    throw std::out_of_range("Index out of bounds in DArray");
  }

  return *reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                (index * _stride));
}

template <typename T> const T &DArray<T>::operator[](uint64 index) const {
  if (index >= _length) {
    FERROR("DArray<T>::operator[]: index out of bounds");
    throw std::out_of_range("Index out of bounds in DArray");
  }

  return *reinterpret_cast<const T *>(
      reinterpret_cast<const char *>(_array.get()) + (index * _stride));
}

// PRIVATE

template <typename T> void DArray<T>::InitializeMemory() {
  uint64 headerSize = DARRAY_FIELD_LENGTH * sizeof(uint64);
  uint64 arraySize = _capacity * _stride;
  _array = std::unique_ptr<T[], decltype(&CustomDeleter)>(
      reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
          headerSize + arraySize, core::memory::MEMORY_TAG_DARRAY)),
      CustomDeleter);

  core::memory::MemoryManager::SetMemory(_array.get(), 0,
                                         headerSize + arraySize);
}

} // namespace containers
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
