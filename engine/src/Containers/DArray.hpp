#ifndef _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
#define _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP

#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Definitions.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>

namespace flatearth {
namespace containers {

template <typename T> class DArray {
public:
  // Constants
  static constexpr uint64 DARRAY_DEFAULT_SIZE = 1;
  static constexpr uchar DARRAY_RESIZE_FACTOR = 2;
  static constexpr uint64 DARRAY_FIELD_LENGTH = 3;

  DArray(uint64 stride = sizeof(T));
  DArray(uint64 capacity, uint64 stride = sizeof(T));
  ~DArray() = default;

  uint64 GetCapacity() const;
  void SetCapacity(uint64 capacity);

  uint64 GetLength() const;
  void SetLength(uint64 length);

  uint64 GetStride() const;
  void SetStride(uint64 stride);

  void Resize();

  void Push(const T &element);
  void Pop();

  void InsertAt(const T &element, uint64 index);
  void PopAt(uint64 index);
  void Clear();

  // Operators
  T &operator[](uint64 index);
  const T &operator[](uint64 index) const;

  // Checkers
  bool IsEmpty() const;

private:
  void InitializeMemory();

  uint64 _capacity;
  uint64 _length;
  uint64 _stride;
  std::unique_ptr<T[],
                  core::memory::CustomDeleter<T, DARRAY_DEFAULT_SIZE,
                                              core::memory::MEMORY_TAG_DARRAY>>
      _array;
};

template <typename T>
DArray<T>::DArray(uint64 stride)
    : _capacity(DARRAY_DEFAULT_SIZE), _length(0), _stride(stride),
      _array(nullptr,
             core::memory::CustomDeleter<T, DARRAY_DEFAULT_SIZE,
                                         core::memory::MEMORY_TAG_DARRAY>()) {
  InitializeMemory();
}

template <typename T>
DArray<T>::DArray(uint64 capacity, uint64 stride)
    : _capacity(capacity), _length(0), _stride(stride),
      _array(nullptr,
             core::memory::CustomDeleter<T, DARRAY_DEFAULT_SIZE,
                                         core::memory::MEMORY_TAG_DARRAY>()) {
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
  // Calculate new capacity
  uint64 newCapacity = _capacity * DARRAY_RESIZE_FACTOR;

  // Determine the total new size to be allocated including header
  uint64 headerSize = DARRAY_FIELD_LENGTH * sizeof(uint64);
  uint64 newArraySize = newCapacity * _stride;
  uint64 totalNewSize = headerSize + newArraySize;

  // Allocate new memory
  T *newMemory = reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
      totalNewSize, core::memory::MEMORY_TAG_DARRAY));
  core::memory::MemoryManager::SetMemory(newMemory, 0, totalNewSize);

  // Move each existing element from old array to new one
  for (uint64 i = 0; i < _length; i++) {
    // Calculate pointers for the current element to old and new arrays
    T *oldElem = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                       i * _stride);
    T *newElem = reinterpret_cast<T *>(reinterpret_cast<char *>(newMemory) +
                                       i * _stride);

    // Use placement new to move-construct the element into the new memory
    new (newElem) T(std::move(*oldElem));

    // Explicitly destruct old element
    oldElem->~T();
  }

  // Once we're done, we need to free the old memory
  uint64 totalOldSize = headerSize + _capacity * _stride;
  core::memory::MemoryManager::Free(_array.get(), totalOldSize,
                                    core::memory::MEMORY_TAG_DARRAY);

  // Update internal state
  _capacity = newCapacity;
  _array.reset(newMemory);
}

template <typename T> void DArray<T>::Push(const T &element) {
  if (_length >= _capacity) {
    Resize();
  }

  // Calculate the destination address of the last element
  T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                  _length * _stride);

  // Construct the new element in place
  new (dest) T(element);
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

  // Shift elements to the right by moving them. Since the destination slots
  // (except for the very end) already contain objects, use assignment.
  // Start from the end and go backwards.
  for (uint64 i = _length; i > index; i--) {
    T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                    (i * _stride));
    T *source = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                      ((i - 1) * _stride));
    *dest = std::move(*source);
  }

  // Now, if index == _length then the slot is uninitialized; otherwise, it is
  // already constructed. In the latter case, we can assign to it.
  T *insertLoc = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                       (index * _stride));
  if (index < _length) {
    // Use copy assignment.
    *insertLoc = element;
  } else {
    // Placement new if inserting at the end.
    new (insertLoc) T(element);
  }
  _length++;
}

template <typename T> void DArray<T>::PopAt(uint64 index) {
  if (index >= _length) {
    FERROR("DArray<T>::PopAt(): attempt to pop at invalid index");
    return;
  }

  // Shift elements to the left.
  for (uint64 i = index; i < _length - 1; i++) {
    T *dest = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                    (i * _stride));
    T *source = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                      ((i + 1) * _stride));
    *dest = std::move(*source);
  }

  // Now the last element is a duplicate; call its destructor.
  T *last = reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                                  ((_length - 1) * _stride));
  last->~T();
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

template <typename T> bool DArray<T>::IsEmpty() const { return _length == 0; }

// PRIVATE

template <typename T> void DArray<T>::InitializeMemory() {
  uint64 headerSize = DARRAY_FIELD_LENGTH * sizeof(uint64);
  uint64 arraySize = _capacity * _stride;
  _array = std::unique_ptr<
      T[], core::memory::CustomDeleter<T, DARRAY_DEFAULT_SIZE,
                                       core::memory::MEMORY_TAG_DARRAY>>(
      reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
          headerSize + arraySize, core::memory::MEMORY_TAG_DARRAY)),
      core::memory::CustomDeleter<T, DARRAY_DEFAULT_SIZE,
                                  core::memory::MEMORY_TAG_DARRAY>());

  core::memory::MemoryManager::SetMemory(_array.get(), 0,
                                         headerSize + arraySize);
}

} // namespace containers
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
