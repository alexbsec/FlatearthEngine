#ifndef _FLATEARTH_ENGINE_STATIC_ARRAY_HPP
#define _FLATEARTH_ENGINE_STATIC_ARRAY_HPP

#include "Core/FeMemory.hpp"
#include "Core/Logger.hpp"
#include "Definitions.hpp"
#include <memory>
#include <stdexcept>
#include <utility>

namespace flatearth {
namespace containers {

template <typename T, uint64 Size> class SArray {
public:
  static constexpr uint64 SARRAY_FIELD_LENGTH = 0;

  SArray();
  ~SArray();

  constexpr uint64 GetSize() const noexcept;
  constexpr bool IsEmpty() const noexcept;

  void Swap(SArray &other) noexcept;

  T &At(uint64 index);
  const T &At(uint64 index) const;

  T *Begin() noexcept;
  const T *Begin() const noexcept;
  T *End() noexcept;
  const T *End() const noexcept;

  // Operators
  T &operator[](uint64 index);
  const T &operator[](uint64 index) const;

private:
  bool _initialized;
  T *GetAddressOf(uint64 index) noexcept;
  const T *GetAddressOf(uint64 index) const noexcept;
  static constexpr uint64 _stride = sizeof(T);
  std::unique_ptr<T[], core::memory::StatelessCustomDeleter<
                           T, Size, core::memory::MEMORY_TAG_ARRAY>>
      _array;
};

template <typename T, uint64 Size> SArray<T, Size>::SArray() {
  if (Size == 0) {
    FERROR("SArray<T, Size>::SArray(): attempt to create a 0 size array");
    throw std::runtime_error("Cannot create 0-sized static array");
  }
  uint64 headerSize = SARRAY_FIELD_LENGTH * sizeof(T);
  uint64 arraySize = _stride * Size;
  _array = std::unique_ptr<T[], core::memory::StatelessCustomDeleter<
                                    T, Size, core::memory::MEMORY_TAG_ARRAY>>(
      reinterpret_cast<T *>(core::memory::MemoryManager::Allocate(
          headerSize + arraySize, core::memory::MEMORY_TAG_ARRAY)),
      core::memory::StatelessCustomDeleter<T, Size,
                                           core::memory::MEMORY_TAG_ARRAY>());

  core::memory::MemoryManager::SetMemory(_array.get(), 0,
                                         headerSize + arraySize);

  for (uint64 i = 0; i < Size; i++) {
    new (GetAddressOf(i)) T();
  }

  _initialized = FeTrue;
}

template <typename T, uint64 Size> SArray<T, Size>::~SArray() {
  if (_initialized) {
    for (uint64 i = 0; i < Size; i++) {
      GetAddressOf(i)->~T();
    }
  }
  _initialized = FeFalse;
}

template <typename T, uint64 Size>
constexpr uint64 SArray<T, Size>::GetSize() const noexcept {
  return Size;
}

template <typename T, uint64 Size>
constexpr bool SArray<T, Size>::IsEmpty() const noexcept {
  return !_initialized;
}

template <typename T, uint64 Size>
void SArray<T, Size>::Swap(SArray &other) noexcept {
  bool tempInit = _initialized;
  _initialized = other._initialized;
  other._initialized = tempInit;
  std::swap(_array, other._array);
}

template <typename T, uint64 Size> T &SArray<T, Size>::At(uint64 index) {
  if (index >= Size) {
    FERROR("SArray<T, Size>::At(): index out of bounds");
    throw std::out_of_range("Index out of bounds in SArray");
  }

  return *GetAddressOf(index);
}

template <typename T, uint64 Size>
const T &SArray<T, Size>::At(uint64 index) const {
  if (index >= Size) {
    FERROR("SArray<T, Size>::At(): index out of bounds");
    throw std::out_of_range("Index out of bounds in SArray");
  }

  return *GetAddressOf(index);
}

template <typename T, uint64 Size> T *SArray<T, Size>::Begin() noexcept {
  return _array.get();
}

template <typename T, uint64 Size>
const T *SArray<T, Size>::Begin() const noexcept {
  return _array.get();
}

template <typename T, uint64 Size> T *SArray<T, Size>::End() noexcept {
  return GetAddressOf(Size);
}

template <typename T, uint64 Size>
const T *SArray<T, Size>::End() const noexcept {
  return GetAddressOf(Size);
}

template <typename T, uint64 Size>
T &SArray<T, Size>::operator[](uint64 index) {
  if (index >= Size) {
    FERROR("SArray<T, Size>::At(): index out of bounds");
    throw std::out_of_range("Index out of bounds in SArray");
  }

  return *GetAddressOf(index);
}

template <typename T, uint64 Size>
const T &SArray<T, Size>::operator[](uint64 index) const {
  if (index >= Size) {
    FERROR("SArray<T, Size>::At(): index out of bounds");
    throw std::out_of_range("Index out of bounds in SArray");
  }

  return *GetAddressOf(index);
}

// Private members
template <typename T, uint64 Size>
T *SArray<T, Size>::GetAddressOf(uint64 index) noexcept {
  return reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                               (index * _stride));
}

template <typename T, uint64 Size>
const T *SArray<T, Size>::GetAddressOf(uint64 index) const noexcept {
  return reinterpret_cast<T *>(reinterpret_cast<char *>(_array.get()) +
                               (index * _stride));
}

} // namespace containers
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_STATIC_ARRAY_HPP
