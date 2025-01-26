#ifndef _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
#define _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP

#include "Definitions.hpp"

#include "Core/FeMemory.hpp"
#include <memory>

namespace flatearth {
namespace containers {

template <typename T> class DArray {
public:
  FEAPI static constexpr uint64 DARRAY_DEFAULT_SIZE = 1;
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

  FEAPI void Push(const T &element);
  FEAPI void Pop();

  FEAPI void InsertAt(const T &element, uint64 index);
  FEAPI void PopAt(uint64 index);
  FEAPI void Clear();

private:
  void InitializeMemory();

  uint64 _capacity;
  uint64 _length;
  uint64 _stride;
  std::unique_ptr<T[], decltype(&core::memory::MemoryManager::Free)> _array;
};

} // namespace containers
} // namespace flatearth

#endif // _FLATEARTH_ENGINE_DYNAMIC_ARRAY_HPP
